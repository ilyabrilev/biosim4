/**
 * UI controls — binds sidebar buttons/sliders to WebSocket commands.
 */
export class Controls {
    constructor(connection) {
        this.connection = connection;

        this.paused = false;
        this.btnPauseResume = document.getElementById('btnPauseResume');
        this.btnPauseResume.addEventListener('click', () => {
            this.connection.send(this.paused ? 'resume' : 'pause');
            this.paused = !this.paused;
            this.btnPauseResume.textContent = this.paused ? 'Resume' : 'Pause';
        });

        document.getElementById('btnRestart').addEventListener('click', () => {
            this.connection.send('restart');
        });

        const speedSlider = document.getElementById('speedSlider');
        const speedVal = document.getElementById('speedVal');
        speedSlider.addEventListener('input', () => {
            speedVal.textContent = speedSlider.value;
            this.connection.send('speed:' + speedSlider.value);
        });

        const mutationInput = document.getElementById('mutationRate');
        document.getElementById('btnMutationOk').addEventListener('click', () => {
            const val = parseFloat(mutationInput.value);
            if (!isNaN(val) && val >= 0 && val <= 1) {
                this.connection.send('setting:pointmutationrate=' + val);
            }
        });

        const stepsInput = document.getElementById('stepsPerGen');
        document.getElementById('btnStepsOk').addEventListener('click', () => {
            const val = parseInt(stepsInput.value, 10);
            if (!isNaN(val) && val > 0) {
                this.connection.send('setting:stepspergeneration=' + val);
            }
        });

        const populationInput = document.getElementById('population');
        document.getElementById('btnPopulationOk').addEventListener('click', () => {
            const val = parseInt(populationInput.value, 10);
            if (!isNaN(val) && val > 0) {
                this.connection.send('setting:population=' + val);
            }
        });

        this.challengeSelect = document.getElementById('challengeSelect');
        this.challengeSelect.addEventListener('change', () => {
            this.connection.send('setting:challenge=' + this.challengeSelect.value);
        });

        this.barrierSelect = document.getElementById('barrierSelect');
        this.barrierSelect.addEventListener('change', () => {
            this.connection.send('setting:barriertype=' + this.barrierSelect.value);
        });

        this.killEnable = document.getElementById('killEnable');
        this.killEnable.addEventListener('change', () => {
            this.connection.send('setting:killenable=' + (this.killEnable.checked ? '1' : '0'));
        });
    }

    applyMeta(meta) {
        if (meta.challenges) {
            this.challengeSelect.innerHTML = '';
            for (const ch of meta.challenges) {
                const opt = document.createElement('option');
                opt.value = ch.value;
                opt.textContent = ch.text;
                opt.title = ch.description;
                this.challengeSelect.appendChild(opt);
            }
        }
        if (meta.barriers) {
            this.barrierSelect.innerHTML = '';
            for (const b of meta.barriers) {
                const opt = document.createElement('option');
                opt.value = b.value;
                opt.textContent = b.text;
                this.barrierSelect.appendChild(opt);
            }
        }
        if (meta.challenge !== undefined) {
            this.challengeSelect.value = meta.challenge;
        }
        if (meta.barrierType !== undefined) {
            this.barrierSelect.value = meta.barrierType;
        }
        if (meta.pointMutationRate !== undefined) {
            document.getElementById('mutationRate').value = meta.pointMutationRate;
        }
        if (meta.stepsPerGeneration !== undefined) {
            document.getElementById('stepsPerGen').value = meta.stepsPerGeneration;
        }
        if (meta.population !== undefined) {
            document.getElementById('population').value = meta.population;
        }
        if (meta.killEnable !== undefined) {
            this.killEnable.checked = meta.killEnable;
        }
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
