
const http = require('http');
const querystring = require('querystring');

// --- CONFIGURATION ---
const CLIENT_ID = '';       // Paste your Client ID
const CLIENT_SECRET = ''; // Paste your Client Secret
const REDIRECT_URI = '';
const PORT = 8888;
const SCOPES = 'user-read-private user-read-email';

const server = http.createServer(async (req, res) => {
    const url = new URL(req.url, `http://${req.headers.host}`);

    if (url.pathname === '/') {
        const authUrl = 'https://accounts.spotify.com/authorize?' +
            querystring.stringify({
                response_type: 'code',
                client_id: CLIENT_ID,
                scope: SCOPES,
                redirect_uri: REDIRECT_URI,
            });

        res.writeHead(302, { Location: authUrl });
        res.end();
        console.log(`[INFO] Redirecting user to Spotify Auth...`);
    } 
    
    else if (url.pathname === '/callback') {
        const code = url.searchParams.get('code');
        const error = url.searchParams.get('error');

        if (error) {
            res.end(`Error: ${error}`);
            console.error(`[ERROR] Spotify returned an error: ${error}`);
            return;
        }

        if (code) {
            console.log(`[INFO] Authorization code received. Exchanging for tokens...`);
            
            try {
                const tokenData = await exchangeCodeForToken(code);
                
                res.writeHead(200, { 'Content-Type': 'text/html' });
                res.end(`
                    <h1>Success!</h1>
                    <p>Check your console for the Refresh Token.</p>
                    <pre>${JSON.stringify(tokenData, null, 2)}</pre>
                `);

                console.log('\n------------------------------------------------');
                console.log('ACCESS TOKEN:', tokenData.access_token);
                console.log('------------------------------------------------');
                console.log('REFRESH TOKEN (Save this!):', tokenData.refresh_token);
                console.log('------------------------------------------------\n');
                
                server.close(() => {
                    console.log('[INFO] Server closed. You can terminate the script.');
                    process.exit(0);
                });

            } catch (err) {
                res.end('Error exchanging code for token. Check console.');
                console.error('[ERROR]', err);
            }
        }
    }
});

async function exchangeCodeForToken(code) {
    const params = new URLSearchParams();
    params.append('grant_type', 'authorization_code');
    params.append('code', code);
    params.append('redirect_uri', REDIRECT_URI);

    const authHeader = 'Basic ' + Buffer.from(CLIENT_ID + ':' + CLIENT_SECRET).toString('base64');

    const response = await fetch('https://accounts.spotify.com/api/token', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
            'Authorization': authHeader
        },
        body: params
    });

    const data = await response.json();
    
    if (data.error) {
        throw new Error(JSON.stringify(data));
    }

    return data;
}


server.listen(PORT, () => {
    console.log(`\n[START] Server running at http://localhost:${PORT}`);
    console.log(`[ACTION] Open your browser and go to http://localhost:${PORT} to start auth flow.\n`);
});