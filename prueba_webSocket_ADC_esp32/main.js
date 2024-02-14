const socket = new WebSocket('ws://192.168.0.245/ws');

// Evento de apertura de la conexión
socket.addEventListener('open', (event) => {
    console.log('Conexión abierta');
});

// Evento de cierre de la conexión
socket.addEventListener('close', (event) => {
    console.log('Conexión cerrada');
});

// Evento de mensaje recibido
socket.addEventListener('message', (event) => {
    console.log('Mensaje recibido:', event.data);
});

// Enviar mensajes para encender y apagar el LED
socket.send('on');  // Encender el LED
socket.send('off'); // Apagar el LED