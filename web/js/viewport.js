/**
 * Pan & zoom viewport for the canvas.
 */
export class Viewport {
    constructor(canvas) {
        this.canvas = canvas;
        this.scale = 4;
        this.offsetX = 0;
        this.offsetY = 0;
        this.onChanged = null;

        this._dragging = false;
        this._dragStartX = 0;
        this._dragStartY = 0;

        this._bindEvents();
    }

    /** Apply the viewport transform to a canvas 2d context. */
    apply(ctx, gridW, gridH) {
        const cw = this.canvas.width;
        const ch = this.canvas.height;
        ctx.translate(this.offsetX + cw / 2, this.offsetY + ch / 2);
        ctx.scale(this.scale, this.scale);
        ctx.translate(-gridW / 2, -gridH / 2);
    }

    _bindEvents() {
        this.canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            const zoomFactor = e.deltaY < 0 ? 1.15 : 1 / 1.15;
            const rect = this.canvas.getBoundingClientRect();
            const mx = e.clientX - rect.left - this.canvas.width / 2 - this.offsetX;
            const my = e.clientY - rect.top - this.canvas.height / 2 - this.offsetY;

            this.offsetX -= mx * (zoomFactor - 1);
            this.offsetY -= my * (zoomFactor - 1);
            this.scale = Math.max(0.5, Math.min(this.scale * zoomFactor, 50));
            this.onChanged?.();
        });

        this.canvas.addEventListener('mousedown', (e) => {
            this._dragging = true;
            this._dragStartX = e.clientX - this.offsetX;
            this._dragStartY = e.clientY - this.offsetY;
        });

        this.canvas.addEventListener('mousemove', (e) => {
            if (!this._dragging) return;
            this.offsetX = e.clientX - this._dragStartX;
            this.offsetY = e.clientY - this._dragStartY;
            this.onChanged?.();
        });

        this.canvas.addEventListener('mouseup', () => { this._dragging = false; });
        this.canvas.addEventListener('mouseleave', () => { this._dragging = false; });
    }
}
