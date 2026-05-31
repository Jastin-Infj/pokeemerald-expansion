import { defineConfig, type Plugin } from "vite";
import react from "@vitejs/plugin-react";
import {
  runApplyPlanFromNode,
  runDryRunPlanFromNode,
  scanProjectFromNode,
} from "./dev/scan-project";

const host = process.env.TAURI_DEV_HOST;

export default defineConfig({
  plugins: [react(), mapScanApiPlugin()],
  clearScreen: false,
  server: {
    host: host || "127.0.0.1",
    port: 1420,
    strictPort: true,
    hmr: host
      ? {
          protocol: "ws",
          host,
          port: 1421,
        }
      : undefined,
    watch: {
      ignored: ["**/src-tauri/**"],
    },
  },
  envPrefix: ["VITE_", "TAURI_ENV_*"],
  build: {
    target:
      process.env.TAURI_ENV_PLATFORM === "windows" ? "chrome105" : "safari13",
    minify: !process.env.TAURI_ENV_DEBUG ? "esbuild" : false,
    sourcemap: Boolean(process.env.TAURI_ENV_DEBUG),
  },
});

function mapScanApiPlugin(): Plugin {
  return {
    name: "map-asset-relinker-scan-api",
    configureServer(server) {
      server.middlewares.use("/api/scan", (request, response) => {
        try {
          const url = new URL(request.url ?? "", "http://127.0.0.1");
          const summary = scanProjectFromNode(url.searchParams.get("root"));
          response.statusCode = 200;
          response.setHeader("Content-Type", "application/json");
          response.end(JSON.stringify(summary));
        } catch (error) {
          response.statusCode = 500;
          response.setHeader("Content-Type", "text/plain");
          response.end(error instanceof Error ? error.message : String(error));
        }
      });
      server.middlewares.use("/api/dry-run", (request, response) => {
        if (request.method !== "POST") {
          response.statusCode = 405;
          response.end("POST required");
          return;
        }

        let body = "";
        request.setEncoding("utf8");
        request.on("data", (chunk) => {
          body += chunk;
        });
        request.on("end", () => {
          try {
            const result = runDryRunPlanFromNode(JSON.parse(body));
            response.statusCode = 200;
            response.setHeader("Content-Type", "application/json");
            response.end(JSON.stringify(result));
          } catch (error) {
            response.statusCode = 500;
            response.setHeader("Content-Type", "text/plain");
            response.end(error instanceof Error ? error.message : String(error));
          }
        });
      });
      server.middlewares.use("/api/apply", (request, response) => {
        if (request.method !== "POST") {
          response.statusCode = 405;
          response.end("POST required");
          return;
        }

        let body = "";
        request.setEncoding("utf8");
        request.on("data", (chunk) => {
          body += chunk;
        });
        request.on("end", () => {
          try {
            const result = runApplyPlanFromNode(JSON.parse(body));
            response.statusCode = 200;
            response.setHeader("Content-Type", "application/json");
            response.end(JSON.stringify(result));
          } catch (error) {
            response.statusCode = 500;
            response.setHeader("Content-Type", "text/plain");
            response.end(error instanceof Error ? error.message : String(error));
          }
        });
      });
    },
  };
}
