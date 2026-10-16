// Safe, headless unit test of the generated offline dashboard: no browser or network.
const fs = require('fs');
const vm = require('vm');
const assert = require('assert');
process.on('uncaughtException', error => { console.error(error.message); process.exit(1); });
const html = fs.readFileSync(process.argv[2], 'utf8').replace(/\r\n/g, '\n');
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
const datasets = JSON.parse(script.split('const data=')[1].split(';\nconst $=')[0]);
assert.equal(nodes.get('scenario').children.length,Number(process.argv[3]||12));
for(let i=0;i<datasets.length;i++) {
  nodes.get('scenario').value=String(i);
  nodes.get('scenario').events.change();
  assert(nodes.get('title').textContent);
  assert(nodes.get('csv').href.endsWith('.csv'));
  nodes.get('cursor').value=String(nodes.get('cursor').max);
  nodes.get('cursor').events.input();
  const hours=datasets[i].rows.at(-1)[0];
  assert.equal(nodes.get('time').textContent,`${Math.floor(hours).toString().padStart(2,'0')}:${Math.floor((hours%1)*60+1e-5).toString().padStart(2,'0')}`);
}
const tankCase=datasets.findIndex(s=>s.id==='tank-full');
if(tankCase>=0) {
 nodes.get('scenario').value=String(tankCase); nodes.get('scenario').events.change();
 nodes.get('cursor').value=String(nodes.get('cursor').max); nodes.get('cursor').events.input();
 assert(nodes.get('warning').innerHTML.includes('megtelt'));
 assert.equal(nodes.get('socket').textContent,'BE');
 assert.equal(nodes.get('compressor').textContent,'ÁLL');
}
console.log(`Dashboard: ${datasets.length} scenarios, charts, parameters and timeline passed (mock DOM, no visual QA).`);
