import { invoke } from "@tauri-apps/api/core";
import { open } from "@tauri-apps/plugin-dialog";
import type {
  DiagnosticLogSnapshot,
  DryRunResult,
  PlanOptions,
  ProjectSummary,
} from "./types";

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

export async function writeDiagnosticEvent(
  event: string,
  details: Record<string, unknown> = {},
): Promise<string | null> {
  if (!window.__TAURI_INTERNALS__) {
    console.info("[diagnostic]", event, details);
    return null;
  }
  try {
    return await invoke<string>("write_diagnostic_event", {
      event: { event, details },
    });
  } catch (err) {
    console.warn("failed to write diagnostic event", event, err);
    return null;
  }
}

export async function readDiagnosticLog(): Promise<DiagnosticLogSnapshot> {
  if (window.__TAURI_INTERNALS__) {
    return invoke<DiagnosticLogSnapshot>("read_diagnostic_log", { lineCount: 160 });
  }
  return {
    path: "Browser development console",
    lines: ["Diagnostics are written to the browser console in Vite mode."],
  };
}

export async function chooseProjectRoot(currentRoot: string): Promise<string | null> {
  if (window.__TAURI_INTERNALS__) {
    await writeDiagnosticEvent("folder_picker_requested", {
      provider: "native",
      currentRoot,
    });
    try {
      const selected = await invoke<string | null>("choose_project_root", {
        currentRoot: currentRoot.trim() || null,
      });
      await writeDiagnosticEvent(
        selected ? "folder_picker_selected" : "folder_picker_cancelled",
        { provider: "native", selectedRoot: selected },
      );
      return selected;
    } catch (nativeError) {
      await writeDiagnosticEvent("folder_picker_native_error", {
        message: nativeError instanceof Error ? nativeError.message : String(nativeError),
      });
      console.warn("native folder picker command failed", nativeError);
      await writeDiagnosticEvent("folder_picker_requested", {
        provider: "plugin",
        currentRoot,
      });
      try {
        const selected = await open({
          title: "Select pokeemerald-expansion Project",
          directory: true,
          multiple: false,
          defaultPath: currentRoot.trim() || undefined,
        });
        if (Array.isArray(selected)) {
          const selectedRoot = selected[0] ?? null;
          await writeDiagnosticEvent(
            selectedRoot ? "folder_picker_selected" : "folder_picker_cancelled",
            { provider: "plugin", selectedRoot },
          );
          return selectedRoot;
        }
        await writeDiagnosticEvent(
          selected ? "folder_picker_selected" : "folder_picker_cancelled",
          { provider: "plugin", selectedRoot: selected },
        );
        return selected;
      } catch (pluginError) {
        await writeDiagnosticEvent("folder_picker_plugin_error", {
          message: pluginError instanceof Error ? pluginError.message : String(pluginError),
        });
        throw pluginError;
      }
    }
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
