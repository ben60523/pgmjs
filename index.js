const pgmjs = require('bindings')('pgmjs');
const math = require('mathjs')
const buf = pgmjs.readPGM('/home/ben/map.pgm')
console.log(math.mean(Array.from(buf)))
console.log(math.max(Array.from(buf)))
console.log(math.min(Array.from(buf)))
