/**
 * WebSocket connection manager with auto-reconnect.
 */
export class Connection {
    constructor(port = 9002) {
        this.port = port;
        this.ws = null;
        this.connected = false;
        this.onMessage = null;
        this.onMeta = null;
        this.onStatusChange = null;
    }

    connect() {
        const host = location.hostname || 'localhost';
        this.ws = new WebSocket(`ws://${host}:${this.port}`);
        this.ws.binaryType = 'arraybuffer';

        this.ws.onopen = () => {
            this.connected = true;
            this.onStatusChange?.(true);
        };

        this.ws.onclose = () => {
            this.connected = false;
            this.onStatusChange?.(false);
            setTimeout(() => this.connect(), 2000);
        };

        this.ws.onerror = () => this.ws.close();

        this.ws.onmessage = (e) => {
            if (typeof e.data === 'string') {
                try {
                    const meta = JSON.parse(e.data);
                    this.onMeta?.(meta);
                } catch (err) { /* ignore malformed text */ }
            } else {
                this.onMessage?.(e.data);
            }
        };
    }

    send(message) {
        if (this.ws && this.connected) {
            this.ws.send(message);
        }
    }
}
