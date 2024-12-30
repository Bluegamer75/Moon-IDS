const express = require('express');
const mysql = require('mysql2');

// Crear la aplicación Express
const app = express();
const port = 3000;  // Puedes cambiar el puerto

// Crear la conexión a la base de datos MySQL
const db = mysql.createConnection({
  host: 'localhost',
  user: 'root', // Cambia esto si es necesario
  password: '',
  database: 'moon', // Cambia esto por el nombre de tu base de datos
});

// Conectar a la base de datos
db.connect(err => {
  if (err) {
    console.error('Error de conexión a la base de datos:', err);
    return;
  }
  console.log('Conectado a la base de datos MySQL');
});

// Crear una ruta para obtener los banners de los puertos
app.get('/api/banners', (req, res) => {
  db.query('SELECT * FROM port_banners', (err, results) => {
    if (err) {
      console.error('Error al obtener los banners:', err);
      return res.status(500).json({ error: 'Error al obtener los banners' });
    }
    res.json(results); // Enviar los banners como respuesta JSON
  });
});

// Servir archivos estáticos (HTML, JS, CSS)
app.use(express.static('moon')); // Aquí se servirán tus archivos HTML

// Iniciar el servidor
app.listen(port, () => {
  console.log(`Servidor escuchando en http://localhost:${port}`);
});
