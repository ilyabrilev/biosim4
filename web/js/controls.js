/**
 * UI controls — binds sidebar buttons/sliders to WebSocket commands.
 */
export class Controls {
    constructor(connection) {
        this.connection = connection;

        document.getElementById('btnPause').addEventListener('click', () => {
            this.connection.send('pause');
        });

        document.getElementById('btnResume').addEventListener('click', () => {
            this.connection.send('resume');
        });

        const speedSlider = document.getElementById('speedSlider');
        const speedVal = document.getElementById('speedVal');
        speedSlider.addEventListener('input', () => {
            speedVal.textContent = speedSlider.value;
            this.connection.send('speed:' + speedSlider.value);
        });
    }

    updateStats(state, fps) {
        if (!state) return;
        document.getElementById('generation').textContent = state.generation;
        document.getElementById('step').textContent = state.simStep;
        document.getElementById('alive').textContent = state.aliveCount;
        document.getElementById('gridSize').textContent = `${state.sizeX}x${state.sizeY}`;
        document.getElementById('fps').textContent = fps;
    }

    setStatus(connected) {
        const el = document.getElementById('status');
        if (connected) {
            el.textContent = 'Connected';
            el.className = 'connected';
        } else {
            el.textContent = 'Disconnected — retrying...';
            el.className = 'disconnected';
        }
    }
}
