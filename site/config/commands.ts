// Single source of truth for kit's command names.
// Rename anything here and it updates everywhere it's used on the site.

export const CLI_NAME = "kit";

export type Command = {
  git: string; // the equivalent git command, for reference
  name: string; // kit's command name
  args?: string;
  description: string;
};

export const COMMANDS: Command[] = [
  { git: "init", name: "new", description: "start a repo here" },
  { git: "add", name: "stage", args: "<file>", description: "queue changes" },
  {
    git: "commit",
    name: "save",
    args: '-m "msg"',
    description: "record a snapshot",
  },
  { git: "status", name: "check", description: "what's changed" },
  { git: "log", name: "history", description: "walk past saves" },
  { git: "diff", name: "compare", description: "line-by-line changes" },
  {
    git: "branch",
    name: "fork",
    args: "<name>",
    description: "fork a line of work",
  },
  {
    git: "checkout",
    name: "switch",
    args: "<name>",
    description: "move between branches",
  },
  {
    git: "merge",
    name: "join",
    args: "<branch>",
    description: "bring branches together",
  },
  { git: "reset", name: "undo", description: "rewind the pointer" },
  { git: "stash", name: "park", description: "set changes aside" },
  {
    git: "remote",
    name: "link",
    args: "<url>",
    description: "connect to a remote",
  },
  {
    git: "clone",
    name: "copy",
    args: "<url>",
    description: "duplicate a repo",
  },
  {
    git: "tag",
    name: "mark",
    args: "<name>",
    description: "flag a point in history",
  },
];
