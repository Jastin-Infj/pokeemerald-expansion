import { invoke } from "@tauri-apps/api/core";
import type { ProjectSummary } from "./types";

declare global {
  interface Window {
    __TAURI_INTERNALS__?: unknown;
  }
}

export async function scanProject(root: string | null): Promise<ProjectSummary> {
  if (window.__TAURI_INTERNALS__) {
    return invoke<ProjectSummary>("scan_project", { root });
  }

  const url = new URL("/api/scan", window.location.origin);
  if (root) {
    url.searchParams.set("root", root);
  }
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(await response.text());
  }
  return (await response.json()) as ProjectSummary;
}
