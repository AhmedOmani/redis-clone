const net = require("net");
const http = require("http");
const fs = require("fs");
const path = require("path");

const REDIS_HOST = process.env.REDIS_HOST || "127.0.0.1";
const REDIS_PORT = parseInt(process.env.REDIS_PORT || "8080");
const HTTP_PORT = parseInt(process.env.HTTP_PORT || "3000");

function sendCommand(args) {
    return new Promise((resolve, reject) => {
        const client = new net.Socket();
        let response = "";

        client.connect(REDIS_PORT, REDIS_HOST, () => {
            let cmd = `*${args.length}\r\n`;
            for (const arg of args) {
                cmd += `$${Buffer.byteLength(arg)}\r\n${arg}\r\n`;
            }
            client.write(cmd);
        });

        client.on("data", (data) => {
            response += data.toString();
            client.destroy();
        });

        client.on("close", () => {
            resolve(parseResp(response));
        });

        client.on("error", (err) => {
            reject(err);
        });

        setTimeout(() => {
            client.destroy();
            reject(new Error("timeout"));
        }, 2000);
    });
}

function parseResp(raw) {
    if (!raw) return null;
    const type = raw[0];
    const body = raw.slice(1).split("\r\n")[0];

    if (type === "+") return { ok: true, value: body };
    if (type === "-") return { ok: false, value: body };
    if (type === ":") return { ok: true, value: parseInt(body) };
    if (type === "$") {
        const len = parseInt(body);
        if (len === -1) return { ok: true, value: null };
        const value = raw.slice(raw.indexOf("\r\n") + 2, raw.lastIndexOf("\r\n"));
        return { ok: true, value };
    }
    return { ok: false, value: raw };
}

const server = http.createServer((req, res) => {
    const url = new URL(req.url, `http://${req.headers.host}`);

    if (url.pathname === "/" || url.pathname === "/index.html") {
        res.writeHead(200, { "Content-Type": "text/html" });
        res.end(fs.readFileSync(path.join(__dirname, "index.html")));
        return;
    }

    if (url.pathname === "/api/set" && req.method === "POST") {
        let body = "";
        req.on("data", (c) => (body += c));
        req.on("end", async () => {
            try {
                const { key, value } = JSON.parse(body);
                const result = await sendCommand(["SET", key, value]);
                res.writeHead(200, { "Content-Type": "application/json" });
                res.end(JSON.stringify(result));
            } catch (e) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ ok: false, value: e.message }));
            }
        });
        return;
    }

    if (url.pathname === "/api/get") {
        const key = url.searchParams.get("key");
        if (!key) {
            res.writeHead(400, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ ok: false, value: "missing key" }));
            return;
        }
        sendCommand(["GET", key])
            .then((result) => {
                res.writeHead(200, { "Content-Type": "application/json" });
                res.end(JSON.stringify(result));
            })
            .catch((e) => {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ ok: false, value: e.message }));
            });
        return;
    }

    res.writeHead(404);
    res.end("not found");
});

server.listen(HTTP_PORT, () => {
    console.log(`Demo server running at http://localhost:${HTTP_PORT}`);
    console.log(`Proxying to redis-clone at ${REDIS_HOST}:${REDIS_PORT}`);
});
