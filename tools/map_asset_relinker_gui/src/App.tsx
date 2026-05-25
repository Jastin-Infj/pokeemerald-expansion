import { useCallback, useEffect, useMemo, useState } from "react";
import { runDryRunPlan, scanProject } from "./backend";
import type { DryRunResult, MapSummary, ProjectSummary } from "./types";

const emptySummary: ProjectSummary = {
  root: "",
  mapCount: 0,
  groupCount: 0,
  layoutCount: 0,
  mapsecCount: 0,
  warningCount: 0,
  maps: [],
  warnings: [],
};

function App() {
  const [summary, setSummary] = useState<ProjectSummary>(emptySummary);
  const [root, setRoot] = useState("");
  const [query, setQuery] = useState("");
  const [selectedName, setSelectedName] = useState<string>("");
  const [status, setStatus] = useState("Idle");
  const [error, setError] = useState<string | null>(null);

  const selected = useMemo(() => {
    return (
      summary.maps.find((map) => map.name === selectedName) ??
      summary.maps[0] ??
      null
    );
  }, [selectedName, summary.maps]);

  const filteredMaps = useMemo(() => {
    const needle = query.trim().toLowerCase();
    if (!needle) {
      return summary.maps;
    }
    return summary.maps.filter((map) => {
      return [
        map.directoryName,
        map.name,
        map.id,
        map.layout,
        map.mapsec,
        map.group ?? "",
      ]
        .join(" ")
        .toLowerCase()
        .includes(needle);
    });
  }, [query, summary.maps]);

  const scan = useCallback(async () => {
    setStatus("Scanning");
    setError(null);
    try {
      const next = await scanProject(root.trim() || null);
      setSummary(next);
      setRoot(next.root);
      setSelectedName((current) => {
        if (next.maps.some((map) => map.name === current)) {
          return current;
        }
        return next.maps[0]?.name ?? "";
      });
      setStatus(`Loaded ${next.mapCount} maps`);
    } catch (err) {
      setStatus("Scan failed");
      setError(err instanceof Error ? err.message : String(err));
    }
  }, [root]);

  useEffect(() => {
    void scan();
  }, []);

  return (
    <main className="appShell">
      <header className="topBar">
        <div className="brandBlock">
          <span className="appMark">MR</span>
          <div>
            <h1>Map Asset Relinker</h1>
            <p>{status}</p>
          </div>
        </div>
        <div className="rootControls">
          <label htmlFor="projectRoot">Project Root</label>
          <input
            id="projectRoot"
            value={root}
            onChange={(event) => setRoot(event.target.value)}
            spellCheck={false}
          />
          <button onClick={scan}>Scan</button>
        </div>
      </header>

      {error ? <div className="errorBanner">{error}</div> : null}

      <section className="metrics" aria-label="Project metrics">
        <Metric label="Maps" value={summary.mapCount} tone="teal" />
        <Metric label="Groups" value={summary.groupCount} tone="blue" />
        <Metric label="Layouts" value={summary.layoutCount} tone="green" />
        <Metric label="Mapsecs" value={summary.mapsecCount} tone="gold" />
        <Metric label="Warnings" value={summary.warningCount} tone="red" />
      </section>

      <section className="workspace">
        <aside className="mapListPane">
          <div className="paneHeader">
            <h2>Maps</h2>
            <input
              aria-label="Filter maps"
              placeholder="Filter"
              value={query}
              onChange={(event) => setQuery(event.target.value)}
            />
          </div>
          <div className="mapList">
            {filteredMaps.map((map) => (
              <button
                key={`${map.directoryName}:${map.id}`}
                className={
                  selected?.name === map.name ? "mapRow isSelected" : "mapRow"
                }
                onClick={() => setSelectedName(map.name)}
              >
                <span>{map.name}</span>
                <small>{map.group ?? "Ungrouped"}</small>
                {map.issues.length > 0 ? (
                  <strong>{map.issues.length}</strong>
                ) : null}
              </button>
            ))}
          </div>
        </aside>

        <section className="detailPane">
          {selected ? (
            <MapDetail map={selected} projectRoot={summary.root} />
          ) : (
            <EmptyState />
          )}
        </section>

        <aside className="auditPane">
          <div className="paneHeader">
            <h2>Audit</h2>
            <span>{summary.warningCount}</span>
          </div>
          <div className="warningList">
            {summary.warnings.length === 0 ? (
              <p className="muted">No warnings.</p>
            ) : (
              summary.warnings.map((warning, index) => (
                <p key={`${warning}:${index}`}>{warning}</p>
              ))
            )}
          </div>
        </aside>
      </section>
    </main>
  );
}

function Metric({
  label,
  value,
  tone,
}: {
  label: string;
  value: number;
  tone: "teal" | "blue" | "green" | "gold" | "red";
}) {
  return (
    <div className={`metric metric-${tone}`}>
      <span>{label}</span>
      <strong>{value.toLocaleString()}</strong>
    </div>
  );
}

function MapDetail({
  map,
  projectRoot,
}: {
  map: MapSummary;
  projectRoot: string;
}) {
  const [newName, setNewName] = useState("");
  const [targetGroup, setTargetGroup] = useState(map.group ?? "");
  const [renameLayout, setRenameLayout] = useState(true);
  const [rewriteScriptLabels, setRewriteScriptLabels] = useState(false);
  const [copied, setCopied] = useState(false);
  const [dryRunStatus, setDryRunStatus] = useState("Not run");
  const [dryRunResult, setDryRunResult] = useState<DryRunResult | null>(null);

  useEffect(() => {
    setNewName(map.name);
    setTargetGroup(map.group ?? "");
    setCopied(false);
    setDryRunStatus("Not run");
    setDryRunResult(null);
  }, [map.name, map.group]);

  const command = useMemo(() => {
    return buildPlanCommand({
      oldName: map.name,
      newName: newName.trim() || map.name,
      targetGroup: targetGroup.trim(),
      renameLayout,
      rewriteScriptLabels,
    });
  }, [map.name, newName, renameLayout, rewriteScriptLabels, targetGroup]);

  const copyCommand = useCallback(async () => {
    await navigator.clipboard.writeText(command);
    setCopied(true);
  }, [command]);

  const runDryRun = useCallback(async () => {
    setDryRunStatus("Running");
    setDryRunResult(null);
    try {
      const result = await runDryRunPlan({
        root: projectRoot,
        oldName: map.name,
        newName: newName.trim() || map.name,
        targetGroup: targetGroup.trim(),
        renameLayout,
        rewriteScriptLabels,
      });
      setDryRunResult(result);
      setDryRunStatus("Dry-run complete");
    } catch (err) {
      setDryRunStatus("Dry-run failed");
      setDryRunResult({
        planPath: "",
        planStdout: "",
        dryRunStdout: "",
        stderr: err instanceof Error ? err.message : String(err),
        command,
      });
    }
  }, [
    command,
    map.name,
    newName,
    projectRoot,
    renameLayout,
    rewriteScriptLabels,
    targetGroup,
  ]);

  return (
    <div className="mapDetail">
      <div className="detailTitle">
        <div>
          <h2>{map.name}</h2>
          <p>{map.id}</p>
        </div>
        <span className={map.issues.length > 0 ? "statusPill warn" : "statusPill"}>
          {map.issues.length > 0 ? `${map.issues.length} issues` : "Clean"}
        </span>
      </div>

      <div className="relationshipGrid">
        <Field label="Directory" value={map.directoryName} />
        <Field label="Group" value={map.group ?? "Ungrouped"} />
        <Field label="Layout" value={map.layout} />
        <Field label="Layout Name" value={map.layoutName ?? "Missing"} />
        <Field label="Mapsec" value={map.mapsec} />
        <Field label="Mapsec Name" value={map.mapsecName ?? "Missing"} />
        <Field label="Mapsec Cell" value={map.mapsecPosition ?? "Unset"} />
        <Field label="Map Type" value={map.mapType} />
        <Field label="Popup" value={map.showMapName ? "show_map_name: true" : "show_map_name: false"} />
      </div>

      <div className="issuePanel">
        <h3>Selected Map Audit</h3>
        {map.issues.length === 0 ? (
          <p className="muted">No selected-map issues.</p>
        ) : (
          map.issues.map((issue) => <p key={issue}>{issue}</p>)
        )}
      </div>

      <div className="planPanel">
        <div className="planHeader">
          <h3>Plan Preview</h3>
          <span>{dryRunStatus}</span>
        </div>
        <div className="planControls">
          <label>
            New Map Name
            <input
              value={newName}
              onChange={(event) => {
                setNewName(event.target.value);
                setCopied(false);
              }}
              spellCheck={false}
            />
          </label>
          <label>
            Target Group
            <input
              value={targetGroup}
              onChange={(event) => {
                setTargetGroup(event.target.value);
                setCopied(false);
              }}
              spellCheck={false}
            />
          </label>
          <label className="toggleLine">
            <input
              type="checkbox"
              checked={renameLayout}
              onChange={(event) => {
                setRenameLayout(event.target.checked);
                setCopied(false);
              }}
            />
            Rename layout with map
          </label>
          <label className="toggleLine">
            <input
              type="checkbox"
              checked={rewriteScriptLabels}
              onChange={(event) => {
                setRewriteScriptLabels(event.target.checked);
                setCopied(false);
              }}
            />
            Rewrite script labels
          </label>
        </div>
        <pre>{command}</pre>
        <div className="actionRail">
          <button onClick={copyCommand}>{copied ? "Copied" : "Copy Command"}</button>
          <button onClick={runDryRun} disabled={!projectRoot || dryRunStatus === "Running"}>
            Run Dry-Run
          </button>
          <button disabled>Apply With Backup</button>
        </div>
        {dryRunResult ? (
          <div className="dryRunOutput">
            <h3>Dry-Run Output</h3>
            <p>{dryRunResult.planPath ? `Plan: ${dryRunResult.planPath}` : "No plan file"}</p>
            <pre>
              {[dryRunResult.dryRunStdout, dryRunResult.stderr]
                .filter(Boolean)
                .join("\n")}
            </pre>
          </div>
        ) : null}
      </div>
    </div>
  );
}

function buildPlanCommand({
  oldName,
  newName,
  targetGroup,
  renameLayout,
  rewriteScriptLabels,
}: {
  oldName: string;
  newName: string;
  targetGroup: string;
  renameLayout: boolean;
  rewriteScriptLabels: boolean;
}) {
  const outName = newName.replace(/[^A-Za-z0-9_]+/g, "_").toLowerCase();
  const args = [
    "tools/map_asset_relinker/map_relink.sh",
    "plan",
    "--map",
    `${oldName}:${newName}`,
  ];
  if (targetGroup) {
    args.push("--to-group", targetGroup);
  }
  if (!renameLayout) {
    args.push("--no-layout-rename");
  }
  if (rewriteScriptLabels) {
    args.push("--rewrite-script-labels");
  }
  args.push("--out", `/tmp/${outName || "map"}_relink.json`);

  return `${args.map(shellQuote).join(" ")}\n${shellQuote(
    "tools/map_asset_relinker/map_relink.sh",
  )} apply --dry-run ${shellQuote(`/tmp/${outName || "map"}_relink.json`)}`;
}

function shellQuote(value: string) {
  if (/^[A-Za-z0-9_./:=-]+$/.test(value)) {
    return value;
  }
  return `'${value.replace(/'/g, "'\\''")}'`;
}

function Field({ label, value }: { label: string; value: string }) {
  return (
    <div className="field">
      <span>{label}</span>
      <strong title={value}>{value}</strong>
    </div>
  );
}

function EmptyState() {
  return (
    <div className="emptyState">
      <h2>No map selected</h2>
      <p>Scan a project to load map linkage data.</p>
    </div>
  );
}

export default App;
