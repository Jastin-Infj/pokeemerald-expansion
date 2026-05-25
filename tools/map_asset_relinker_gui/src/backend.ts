import { invoke } from "@tauri-apps/api/core";
import type { DryRunResult, PlanOptions, ProjectSummary } from "./types";

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

export async function runDryRunPlan(options: PlanOptions): Promise<DryRunResult> {
  if (window.__TAURI_INTERNALS__) {
    return invoke<DryRunResult>("run_plan_dry_run", { options });
  }

  const response = await fetch("/api/dry-run", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(options),
  });
  if (!response.ok) {
    throw new Error(await response.text());
  }
  return (await response.json()) as DryRunResult;
}
