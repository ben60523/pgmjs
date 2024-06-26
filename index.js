const pgmjs = require('bindings')('pgmjs');

module.exports = {
  /**
   * 
   * @param {String} fp: file path 
   * @returns Array [data, w, h]
   */
  readPGM: function (fp) {
      return pgmjs.readPGM(fp);
  },
  /**
   * free all resources
   */
  freeObjs: function () {
    return pgmjs.freeObjs();
  },
  /**
   * 
   * @param {Number} occ_th 
   * @param {Number} free_th 
   */
  loadMap: function(occ_th, free_th, resolution) {
    return pgmjs.loadMap(occ_th, free_th, resolution);
  },

  navigation: function(cx, cy, cz, gx, gy) {
    return pgmjs.navigation(cx, cy, cz, gx, gy);
  }
}
