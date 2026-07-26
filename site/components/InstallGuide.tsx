"use client";

import { useState } from "react";
import { CLI_NAME } from "@/config/commands";

type OS = "mac" | "linux" | "windows";

const TABS: { id: OS; label: string }[] = [
  { id: "mac", label: "macOS" },
  { id: "linux", label: "Linux" },
  { id: "windows", label: "Windows" },
];

export default function InstallGuide() {
  const [os, setOs] = useState<OS>("mac");

  return (
    <div className="overflow-hidden rounded-lg border border-[var(--line)]">
      <div className="flex border-b border-[var(--line)] bg-[var(--bg-raised)]">
        {TABS.map((t) => (
          <button
            key={t.id}
            onClick={() => setOs(t.id)}
            className={`px-4 py-3 text-xs uppercase tracking-widest transition-colors ${
              os === t.id
                ? "text-[var(--ember-2)]"
                : "text-[var(--ink-dim)] hover:text-[var(--ink)]"
            }`}
          >
            {t.label}
          </button>
        ))}
      </div>

      <div className="p-5">
        {os === "mac" && <MacSteps />}
        {os === "linux" && <LinuxSteps />}
        {os === "windows" && <WindowsSteps />}
      </div>
    </div>
  );
}

function Step({
  n,
  title,
  children,
}: {
  n: number;
  title: string;
  children: React.ReactNode;
}) {
  return (
    <li className="flex gap-4">
      <span className="font-display shrink-0 text-sm font-bold text-[var(--ember-2)]">
        {String(n).padStart(2, "0")}
      </span>
      <div className="flex-1">
        <p className="text-sm text-[var(--ink)]">{title}</p>
        <div className="mt-2">{children}</div>
      </div>
    </li>
  );
}

function Code({ children }: { children: string }) {
  return (
    <pre className="overflow-x-auto rounded-md border border-[var(--line)] bg-[var(--bg-raised)] p-3 text-xs leading-relaxed">
      <code className="text-[var(--ink)]">{children}</code>
    </pre>
  );
}

function MacSteps() {
  return (
    <ol className="flex flex-col gap-6">
      <Step
        n={1}
        title={`Unzip and move the ${CLI_NAME} binary somewhere permanent`}
      >
        <Code>{`mkdir -p ~/bin\nmv bin/${CLI_NAME} ~/bin/${CLI_NAME}\nchmod +x ~/bin/${CLI_NAME}`}</Code>
      </Step>
    </ol>
  );
}

function LinuxSteps() {
  return (
    <ol className="flex flex-col gap-6">
      <Step
        n={1}
        title={`Unzip and move the ${CLI_NAME} binary somewhere permanent`}
      >
        <Code>{`mkdir -p ~/.local/bin\nmv bin/${CLI_NAME} ~/.local/bin/${CLI_NAME}\nchmod +x ~/.local/bin/${CLI_NAME}`}</Code>
      </Step>
    </ol>
  );
}

function WindowsSteps() {
  return (
    <ol className="flex flex-col gap-6">
      <Step
        n={1}
        title={`Unzip the download, then move bin/${CLI_NAME}.exe somewhere permanent`}
      >
        <p className="text-xs text-[var(--ink-dim)]">
          e.g.{" "}
          <code className="text-[var(--ink)]">
            C:\Tools\{CLI_NAME}\{CLI_NAME}.exe
          </code>
        </p>
      </Step>
    </ol>
  );
}
