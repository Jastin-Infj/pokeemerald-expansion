export type MapSummary = {
  directoryName: string;
  name: string;
  id: string;
  layout: string;
  layoutName: string | null;
  mapsec: string;
  mapsecName: string | null;
  mapsecPosition: string | null;
  mapType: string;
  showMapName: boolean;
  group: string | null;
  groupCount: number;
  issues: string[];
};

export type ProjectSummary = {
  root: string;
  mapCount: number;
  groupCount: number;
  layoutCount: number;
  mapsecCount: number;
  warningCount: number;
  maps: MapSummary[];
  warnings: string[];
};

export type PlanOptions = {
  root: string;
  oldName: string;
  newName: string;
  targetGroup: string;
  renameMapsecFrom: string;
  renameMapsecTo: string;
  newMapsecName: string;
  renameLayout: boolean;
  rewriteScriptLabels: boolean;
};

export type DryRunResult = {
  planPath: string;
  planStdout: string;
  dryRunStdout: string;
  stderr: string;
  command: string;
};
