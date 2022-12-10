const nvencDevice = require('./index.js');

console.log(nvencDevice.supports('H264'));
console.log(nvencDevice.supports('HEVC'));
console.log(nvencDevice.supports('AV1'));