let isWin = process.platform === "win32";
const execSync = require('child_process').execSync;
const path = require('path');

const binary = isWin ? path.resolve(__dirname, "./build/Release/nvenc_codecs_node.exe") : path.resolve(__dirname, "./build/Release/nvenc_codecs_node")
class nvencDevice {
  static supports(codec) {
    const cmd = `"${binary}"`;
    try {
      let checkCodecs = execSync(cmd, { windowsHide: true, encoding: "utf8" });
      if (checkCodecs.includes(codec)) {
        return true;
      } else if (checkCodecs.includes('Driver does not support the reqired nvenc API version')) {
        console.error('checkCodecs');
        return checkCodecs;
      } else {
        return false;
      }
    } catch (error) {
      console.error(error);
    }
  }
}

module.exports = nvencDevice;