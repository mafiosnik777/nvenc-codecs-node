# nvenc-codecs-node
Using NVIDIA Video Codec SDK (nvEncodeAPI.h) to check encoder capatibilty of current GPU

# Installation

```npm install @mafiosnik/nvenc-codecs```

# Basic Usage

```js
const nvencDevice = require("@mafiosnik/nvenc-codecs");

//Choose to test for either 'H264', 'HEVC', 'AV1'
console.log(nvencDevice.supports('AV1'));
// returns "true" on a RTX 4090
```
