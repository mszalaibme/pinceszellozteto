// Safe, headless unit test of the generated offline dashboard: no browser or network.
const fs = require('fs');
const vm = require('vm');
const assert = require('assert');
const html = fs.readFileSync(process.argv[2], 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];
const nodes = new Map();
const context2d = new Proxy({}, {get: () => () => {}});
class Element {
  constructor() { this.value='0'; this.style={}; this.events={}; this.children=[]; this.textContent=''; }
  append(node) { this.children.push(node); }
  addEventListener(name, handler) { this.events[name]=handler; }
  querySelector() { return new Element(); }
  getBoundingClientRect() { return {width:400,height:150,left:0}; }
  getContext() { return context2d; }
}
const document = {
  getElementById: id => { if(!nodes.has(id)) nodes.set(id,new Element()); return nodes.get(id); },
  createElement: () => new Element(),
};
const env = {document, window:{devicePixelRatio:1,addEventListener(){}}, setInterval:()=>1,clearInterval(){}, console};
vm.createContext(env);
vm.runInContext(script,env,{timeout:10000});
assert.equal(nodes.get('scenario').children.length,12);
for(let i=0;i<12;i++) {
  nodes.get('scenario').value=String(i);
  nodes.get('scenario').events.change();
  assert(nodes.get('title').textContent);
  assert(nodes.get('csv').href.endsWith('.csv'));
  nodes.get('cursor').value=String(nodes.get('cursor').max);
  nodes.get('cursor').events.input();
  assert.equal(nodes.get('time').textContent,'24:00');
}
nodes.get('scenario').value='4'; nodes.get('scenario').events.change();
nodes.get('cursor').value='1440'; nodes.get('cursor').events.input();
assert(nodes.get('warning').innerHTML.includes('megtelt'));
assert.equal(nodes.get('socket').textContent,'BE');
assert.equal(nodes.get('compressor').textContent,'ÁLL');
console.log('Dashboard: 12 scenarios, chart calls, timeline and full-tank distinction passed (mock DOM, no visual QA).');
