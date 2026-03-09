import { parseFrame } from './protocol.js';
import { Connection } from './connection.js';
import { Viewport } from './viewport.js';
import { Renderer } from './renderer.js';
import { Controls } from './controls.js';

const canvas = document.getElementById('canvas');

const connection = new Connection(9002);
const viewport = new Viewport(canvas);
const renderer = new Renderer(canvas, viewport);
const controls = new Controls(connection);

let state = null;

connection.onStatusChange = (connected) => controls.setStatus(connected);
connection.onMeta = (meta) => controls.applyMeta(meta);

connection.onMessage = (data) => {
    state = parseFrame(data);
    renderer.render(state);
    controls.updateStats(state, renderer.fps);
};

viewport.onChanged = () => renderer.render(state);
window.addEventListener('resize', () => renderer.render(state));

connection.connect();
