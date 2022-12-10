# nvenc-codecs-node
Using NVIDIA Video Codec SDK (nvEncodeAPI.h) to check encoder capatibilty of current GPU

# Installation

```npm install @mafiosnik777/nvenc-codecs@1.0.0```

# Basic Usage

```js
const nvencCodecs = require("@mafiosnik/nvenc-codecs");

//Choose to test for either 'H264', 'HEVC', 'AV1'
console.log(nvencDevice.supports('AV1'));
// returns "true" on a RTX 4090
```
