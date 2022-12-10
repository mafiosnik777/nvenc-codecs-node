#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if defined(__WINDOWS__)
#include <sys/param.h>
#endif

#include <ffnvcodec/dynlink_loader.h>

static CudaFunctions *cu;
static NvencFunctions *nv;
static NV_ENCODE_API_FUNCTION_LIST nv_funcs;

static int check_cu(CUresult err, const char *func)
{
  const char *err_name;
  const char *err_string;

  if (err == CUDA_SUCCESS) {
    return 0;
  }

  cu->cuGetErrorName(err, &err_name);
  cu->cuGetErrorString(err, &err_string);

  fprintf(stderr, "%s failed", func);
  if (err_name && err_string) {
    fprintf(stderr, " -> %s: %s", err_name, err_string);
  }
  fprintf(stderr, "\n");

  return -1;
}

#define CHECK_CU(x) { int ret = check_cu((x), #x); if (ret != 0) { return ret; } }

static const struct {
    NVENCSTATUS nverr;
    int         averr;
    const char *desc;
} nvenc_errors[] = {
    { NV_ENC_SUCCESS,                       0, "success"                  },
    { NV_ENC_ERR_NO_ENCODE_DEVICE,         -1, "no encode device"         },
    { NV_ENC_ERR_UNSUPPORTED_DEVICE,       -1, "unsupported device"       },
    { NV_ENC_ERR_INVALID_ENCODERDEVICE,    -1, "invalid encoder device"   },
    { NV_ENC_ERR_INVALID_DEVICE,           -1, "invalid device"           },
    { NV_ENC_ERR_DEVICE_NOT_EXIST,         -1, "device does not exist"    },
    { NV_ENC_ERR_INVALID_PTR,              -1, "invalid ptr"              },
    { NV_ENC_ERR_INVALID_EVENT,            -1, "invalid event"            },
    { NV_ENC_ERR_INVALID_PARAM,            -1, "invalid param"            },
    { NV_ENC_ERR_INVALID_CALL,             -1, "invalid call"             },
    { NV_ENC_ERR_OUT_OF_MEMORY,            -1, "out of memory"            },
    { NV_ENC_ERR_ENCODER_NOT_INITIALIZED,  -1, "encoder not initialized"  },
    { NV_ENC_ERR_UNSUPPORTED_PARAM,        -1, "unsupported param"        },
    { NV_ENC_ERR_LOCK_BUSY,                -1, "lock busy"                },
    { NV_ENC_ERR_NOT_ENOUGH_BUFFER,        -1, "not enough buffer"        },
    { NV_ENC_ERR_INVALID_VERSION,          -1, "invalid version"          },
    { NV_ENC_ERR_MAP_FAILED,               -1, "map failed"               },
    { NV_ENC_ERR_NEED_MORE_INPUT,          -1, "need more input"          },
    { NV_ENC_ERR_ENCODER_BUSY,             -1, "encoder busy"             },
    { NV_ENC_ERR_EVENT_NOT_REGISTERD,      -1, "event not registered"     },
    { NV_ENC_ERR_GENERIC,                  -1, "generic error"            },
    { NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY,  -1, "incompatible client key"  },
    { NV_ENC_ERR_UNIMPLEMENTED,            -1, "unimplemented"            },
    { NV_ENC_ERR_RESOURCE_REGISTER_FAILED, -1, "resource register failed" },
    { NV_ENC_ERR_RESOURCE_NOT_REGISTERED,  -1, "resource not registered"  },
    { NV_ENC_ERR_RESOURCE_NOT_MAPPED,      -1, "resource not mapped"      },
};

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
static int nvenc_map_error(NVENCSTATUS err, const char **desc)
{
    int i;
    for (i = 0; i < FF_ARRAY_ELEMS(nvenc_errors); i++) {
        if (nvenc_errors[i].nverr == err) {
            if (desc)
                *desc = nvenc_errors[i].desc;
            return nvenc_errors[i].averr;
        }
    }
    if (desc)
        *desc = "unknown error";
    return -1;
}

static int check_nv(NVENCSTATUS err, const char *func)
{
  const char *err_string;

  if (err == NV_ENC_SUCCESS) {
    return 0;
  }

  nvenc_map_error(err, &err_string);

  fprintf(stderr, "%s failed", func);
  if (err_string) {
    fprintf(stderr, " -> %s", err_string);
  }
  fprintf(stderr, "\n");

  return -1;
}

#define CHECK_NV(x) { int ret = check_nv((x), #x); if (ret != 0) { return ret; } }

#define NVENCAPI_CHECK_VERSION(major, minor) \
    ((major) < NVENCAPI_MAJOR_VERSION || ((major) == NVENCAPI_MAJOR_VERSION && (minor) <= NVENCAPI_MINOR_VERSION))

static void nvenc_print_driver_requirement()
{
#if NVENCAPI_CHECK_VERSION(9, 1)
# if defined(_WIN32) || defined(__CYGWIN__)
    const char *minver = "436.15";
# else
    const char *minver = "435.21";
# endif
#elif NVENCAPI_CHECK_VERSION(8, 1)
# if defined(_WIN32) || defined(__CYGWIN__)
    const char *minver = "390.77";
# else
    const char *minver = "390.25";
# endif
#else
# if defined(_WIN32) || defined(__CYGWIN__)
    const char *minver = "378.66";
# else
    const char *minver = "378.13";
# endif
#endif
    printf("The minimum required Nvidia driver for nvenc is %s or newer\n", minver);
}

static int nvenc_load_libraries()
{
  uint32_t nvenc_max_ver;
  int ret;

  ret = cuda_load_functions(&cu, NULL);
  if (ret < 0)
    return ret;

  ret = nvenc_load_functions(&nv, NULL);
  if (ret < 0) {
    nvenc_print_driver_requirement();
    return ret;
  }

  CHECK_NV(nv->NvEncodeAPIGetMaxSupportedVersion(&nvenc_max_ver));

  printf("Loaded Nvenc version %d.%d\n", nvenc_max_ver >> 4, nvenc_max_ver & 0xf);

  if ((NVENCAPI_MAJOR_VERSION << 4 | NVENCAPI_MINOR_VERSION) > nvenc_max_ver) {
    printf("Driver does not support the required nvenc API version. "
           "Required: %d.%d Found: %d.%d\n",
           NVENCAPI_MAJOR_VERSION, NVENCAPI_MINOR_VERSION,
           nvenc_max_ver >> 4, nvenc_max_ver & 0xf);
    nvenc_print_driver_requirement();
    return -1;
  }

  nv_funcs.version = NV_ENCODE_API_FUNCTION_LIST_VER;

  CHECK_NV(nv->NvEncodeAPICreateInstance(&nv_funcs));

  printf("Nvenc initialized successfully\n");

  return 0;
}

static int print_codecs(void *encoder)
{
  uint32_t count = 0;
  CHECK_NV(nv_funcs.nvEncGetEncodeGUIDCount(encoder, &count));

  GUID *guids = malloc(count * sizeof(GUID));
  if (!guids) {
    return -1;
  }

  CHECK_NV(nv_funcs.nvEncGetEncodeGUIDs(encoder, guids, count, &count));
  for (int i = 0; i < count; i++) {
    if (memcmp(&guids[i], &NV_ENC_CODEC_H264_GUID, 16) == 0) {
      printf("H264\n");
    } else if (memcmp(&guids[i], &NV_ENC_CODEC_HEVC_GUID, 16) == 0) {
      printf("HEVC\n");
    } else if (memcmp(&guids[i], &NV_ENC_CODEC_AV1_GUID, 16) == 0) {
      printf("AV1");
    }
  }

  free(guids);

  return 0;
}



static int print_nvenc_capabilities(CUcontext cuda_ctx)
{
  NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS params = { 0 };
  void *nvencoder;

  params.version    = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
  params.apiVersion = NVENCAPI_VERSION;
  params.device     = cuda_ctx;
  params.deviceType = NV_ENC_DEVICE_TYPE_CUDA;

  CHECK_NV(nv_funcs.nvEncOpenEncodeSessionEx(&params, &nvencoder));

  CHECK_NV(print_codecs(nvencoder));

  CHECK_NV(nv_funcs.nvEncDestroyEncoder(nvencoder));

  return 0;
}


int main(int argc, char *argv[])
{
  CUcontext cuda_ctx;
  CUcontext dummy;
  int ret;

  ret = nvenc_load_libraries();
  if (ret < 0) {
    return ret;
  }

  CHECK_CU(cu->cuInit(0));
  int count;
  CHECK_CU(cu->cuDeviceGetCount(&count));

  for (int i = 0; i < count; i++) {
    CUdevice dev;
    CHECK_CU(cu->cuDeviceGet(&dev, i));

    char name[255];
    CHECK_CU(cu->cuDeviceGetName(name, 255, dev));
    printf("Device %d: %s\n", i, name);

    CHECK_CU(cu->cuCtxCreate(&cuda_ctx, CU_CTX_SCHED_BLOCKING_SYNC, dev));
    print_nvenc_capabilities(cuda_ctx);
    printf("\n");
    cu->cuCtxPopCurrent(&dummy);
  }

  nvenc_free_functions(&nv);
  cuda_free_functions(&cu);

  return 0;
}
