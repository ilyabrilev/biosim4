/**
 * Binary frame parser for the biosim4 WebSocket protocol.
 *
 * Frame format (little-endian):
 *   Header (16 bytes):
 *     u32 simStep, u32 generation, u16 sizeX, u16 sizeY, u32 aliveCount
 *   Per alive individual (7 bytes):
 *     u16 x, u16 y, u8 r, u8 g, u8 b
 *   Barriers:
 *     u32 count, then per barrier: u16 x, u16 y
 *   Shapes:
 *     u32 count, then per shape: u8 type + type-specific data
 */
export function parseFrame(buffer) {
    const view = new DataView(buffer);
    let offset = 0;

    const read = {
        u32() { const v = view.getUint32(offset, true); offset += 4; return v; },
        u16() { const v = view.getUint16(offset, true); offset += 2; return v; },
        u8()  { const v = view.getUint8(offset); offset += 1; return v; },
        f32() { const v = view.getFloat32(offset, true); offset += 4; return v; },
    };

    const simStep = read.u32();
    const generation = read.u32();
    const sizeX = read.u16();
    const sizeY = read.u16();
    const aliveCount = read.u32();

    const individuals = [];
    for (let i = 0; i < aliveCount; i++) {
        individuals.push({
            x: read.u16(),
            y: read.u16(),
            r: read.u8(),
            g: read.u8(),
            b: read.u8(),
        });
    }

    const barrierCount = read.u32();
    const barriers = [];
    for (let i = 0; i < barrierCount; i++) {
        barriers.push({ x: read.u16(), y: read.u16() });
    }

    const shapeCount = read.u32();
    const shapes = [];
    for (let i = 0; i < shapeCount; i++) {
        const type = read.u8();
        if (type === 0) {
            shapes.push({
                type: 'circle',
                radius: read.f32(), x: read.f32(), y: read.f32(),
                r: read.u8(), g: read.u8(), b: read.u8(), a: read.u8(),
            });
        } else if (type === 1) {
            shapes.push({
                type: 'rect',
                x: read.f32(), y: read.f32(), width: read.f32(), height: read.f32(),
                r: read.u8(), g: read.u8(), b: read.u8(), a: read.u8(),
            });
        } else if (type === 2) {
            shapes.push({
                type: 'line',
                x1: read.f32(), y1: read.f32(), x2: read.f32(), y2: read.f32(),
                r: read.u8(), g: read.u8(), b: read.u8(), a: read.u8(),
            });
        }
    }

    return { simStep, generation, sizeX, sizeY, aliveCount, individuals, barriers, shapes };
}
