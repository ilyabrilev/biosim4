/**
 * Canvas renderer for simulation state.
 */
export class Renderer {
    constructor(canvas, viewport) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.viewport = viewport;
        this.container = canvas.parentElement;

        this._frameCount = 0;
        this._lastFpsTime = performance.now();
        this._fps = 0;
    }

    get fps() { return this._fps; }

    render(state) {
        if (!state) return;

        const { sizeX, sizeY, individuals, barriers, shapes } = state;
        this._fitCanvas();

        const ctx = this.ctx;
        const cw = this.canvas.width;
        const ch = this.canvas.height;

        ctx.fillStyle = '#000';
        ctx.fillRect(0, 0, cw, ch);

        ctx.save();
        this.viewport.apply(ctx, sizeX, sizeY);

        this._drawShapes(ctx, shapes);
        this._drawBarriers(ctx, barriers, sizeY);
        this._drawIndividuals(ctx, individuals, sizeY);

        ctx.restore();
        this._updateFps();
    }

    _fitCanvas() {
        const cw = this.container.clientWidth;
        const ch = this.container.clientHeight;
        if (this.canvas.width !== cw || this.canvas.height !== ch) {
            this.canvas.width = cw;
            this.canvas.height = ch;
        }
    }

    _drawIndividuals(ctx, individuals, sizeY) {
        for (const ind of individuals) {
            ctx.fillStyle = `rgb(${ind.r},${ind.g},${ind.b})`;
            ctx.fillRect(ind.x, sizeY - ind.y - 1, 1, 1);
        }
    }

    _drawBarriers(ctx, barriers, sizeY) {
        ctx.fillStyle = '#888';
        for (const b of barriers) {
            ctx.fillRect(b.x, sizeY - b.y - 1, 1, 1);
        }
    }

    _drawShapes(ctx, shapes) {
        const scale = this.viewport.scale;
        for (const s of shapes) {
            const alpha = (s.a / 255).toFixed(2);
            const color = `rgba(${s.r},${s.g},${s.b},${alpha})`;

            if (s.type === 'circle') {
                ctx.beginPath();
                ctx.arc(s.x + s.radius, s.y + s.radius, s.radius, 0, Math.PI * 2);
                ctx.strokeStyle = color;
                ctx.lineWidth = 1 / scale;
                ctx.stroke();
            } else if (s.type === 'rect') {
                ctx.fillStyle = color;
                ctx.fillRect(s.x, s.y, s.width, s.height);
            } else if (s.type === 'line') {
                ctx.beginPath();
                ctx.moveTo(s.x1, s.y1);
                ctx.lineTo(s.x2, s.y2);
                ctx.strokeStyle = color;
                ctx.lineWidth = 1 / scale;
                ctx.stroke();
            }
        }
    }

    _updateFps() {
        this._frameCount++;
        const now = performance.now();
        if (now - this._lastFpsTime >= 1000) {
            this._fps = this._frameCount;
            this._frameCount = 0;
            this._lastFpsTime = now;
        }
    }
}
