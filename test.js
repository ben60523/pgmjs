const fs = require('fs');
const math = require('mathjs');
const jsyaml = require('js-yaml');
const { readPGM, loadMap, freeObjs, navigation } = require('./index');

console.log(readPGM('pgm/map.pgm'));

const yaml = fs.readFileSync('pgm/map.yaml');
const yaml_obj = jsyaml.load(yaml);
// const occ = loadMap(Number(yaml_obj.occupied_thresh), Number(yaml_obj.free_thresh), Number(yaml_obj.resolution))
// const occ_arr = [...new Int32Array(occ)]
// console.log(`Max: ${math.max(occ_arr)}, Min: ${math.min(occ_arr)}, Mean: ${math.mean(occ_arr)}, Len: ${occ_arr.length}`)
console.log(navigation(190, 318, 0, 190, 150));
console.log(' ');
console.log(navigation(190, 318, 0, 385, 181));
console.log(' ');
console.log(navigation(190, 318, 0, 350, 113));
console.log(' ');
console.log(navigation(190, 318, 0, 220, 318));

freeObjs();