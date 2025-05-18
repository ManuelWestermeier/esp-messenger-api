const WebSocket = require('ws');

const SERVER_URL = 'ws://localporn.duckdns.org:38787/ws'; // Your WS URL here
const TOTAL_CLIENTS = 10;  // Number of clients to try opening
const MESSAGE_INTERVAL_MS = 5000; // Interval to send messages

let connectedCount = 0;
let failedCount = 0;
let clients = [];

console.log(`Starting test: connecting ${TOTAL_CLIENTS} clients to ${SERVER_URL}`);

for (let i = 0; i < TOTAL_CLIENTS; i++) {
    try {
        const ws = new WebSocket(SERVER_URL);

        ws.on('open', () => {
            connectedCount++;
            console.log(`Client #${i} connected (${connectedCount} total)`);

            // Optionally send a message every few seconds
            ws.interval = setInterval(() => {
                if (ws.readyState === WebSocket.OPEN) {
                    ws.send(`Hello from client #${i}`);
                }
            }, MESSAGE_INTERVAL_MS);
        });

        ws.on('message', (msg) => {
            // Uncomment to log received messages (will be very verbose with many clients)
            // console.log(`Client #${i} received: ${msg}`);
        });

        ws.on('error', (err) => {
            failedCount++;
            console.log(`Client #${i} error: ${err.message}`);
        });

        ws.on('close', () => {
            console.log(`Client #${i} disconnected`);
            clearInterval(ws.interval);
        });

        clients.push(ws);
    } catch (e) {
        failedCount++;
        console.log(`Client #${i} exception: ${e.message}`);
    }
}

// Periodically print stats
setInterval(() => {
    console.log(`Connected: ${connectedCount}, Failed: ${failedCount}, Total attempted: ${TOTAL_CLIENTS}`);
}, 10000);
