const pgmjs = require('bindings')('pgmjs');
const math = require('mathjs')

module.exports = {
  readPGM: function (fp) {
      return pgmjs.readPGM(fp);
  },
}
