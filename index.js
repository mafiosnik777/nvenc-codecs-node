let isWin = process.platform === "win32";
const execSync = require('child_process').execSync;

const binary = isWin ? ".\\build\\Release\\nvenc_codecs_node.exe" : "./build/Release/nvenc_codecs_node"

class nvencDevice {
  static supports(codec) {
    const cmd = `"${binary}"`;
    try {
      let checkCodecs = execSync(cmd, { windowsHide: true, encoding: "utf8" });
      if (checkCodecs.includes(codec)) {
        return true;
      } else {
        return false;
      }
    } catch (error) {
      console.error(error);
    }
  }
}

module.exports = nvencDevice;

