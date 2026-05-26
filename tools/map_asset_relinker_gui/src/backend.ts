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

export function isDesktopApp() {
  return Boolean(window.__TAURI_INTERNALS__);
}

export async function chooseProjectRoot(currentRoot: string): Promise<string | null> {
  if (window.__TAURI_INTERNALS__) {
    const { open } = await import("@tauri-apps/plugin-dialog");
    const selected = await open({
      title: "Select pokeemerald-expansion Project",
      directory: true,
      multiple: false,
      defaultPath: currentRoot || undefined,
    });
    if (Array.isArray(selected)) {
      return selected[0] ?? null;
    }
    return selected;
  }

  const selected = window.prompt("Project root", currentRoot);
  return selected?.trim() || null;
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

export async function runApplyPlan(options: PlanOptions): Promise<DryRunResult> {
  if (window.__TAURI_INTERNALS__) {
    return invoke<DryRunResult>("run_plan_apply", { options });
  }

  const response = await fetch("/api/apply", {
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
