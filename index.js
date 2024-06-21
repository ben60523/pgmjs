const pgmjs = require('bindings')('pgmjs');
const math = require('mathjs')

module.exports = {
  /**
   * 
   * @param {String} fp: file path 
   * @returns Array [data, w, h]
   */
  readPGM: function (fp) {
      return pgmjs.readPGM(fp);
  },
}
