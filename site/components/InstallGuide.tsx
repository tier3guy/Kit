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
        <Code>{`mkdir -p ~/bin\nmv ${CLI_NAME} ~/bin/${CLI_NAME}\nchmod +x ~/bin/${CLI_NAME}`}</Code>
      </Step>
      <Step n={2} title="Add that folder to your PATH">
        <p className="mb-2 text-xs text-[var(--ink-dim)]">
          zsh is the default shell on modern macOS — edit{" "}
          <code className="text-[var(--ink)]">~/.zshrc</code> (use{" "}
          <code className="text-[var(--ink)]">~/.bash_profile</code> if
          you&apos;re on bash).
        </p>
        <Code>{`echo 'export PATH="$HOME/bin:$PATH"' >> ~/.zshrc\nsource ~/.zshrc`}</Code>
      </Step>
      <Step n={3} title="Verify it's on PATH">
        <Code>{`${CLI_NAME} status`}</Code>
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
        <Code>{`mkdir -p ~/.local/bin\nmv ${CLI_NAME} ~/.local/bin/${CLI_NAME}\nchmod +x ~/.local/bin/${CLI_NAME}`}</Code>
      </Step>
      <Step n={2} title="Add that folder to your PATH">
        <p className="mb-2 text-xs text-[var(--ink-dim)]">
          Most distros already include{" "}
          <code className="text-[var(--ink)]">~/.local/bin</code> — if{" "}
          <code className="text-[var(--ink)]">which {CLI_NAME}</code> comes up
          empty after step 3, add it explicitly.
        </p>
        <Code>{`echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc\nsource ~/.bashrc`}</Code>
      </Step>
      <Step n={3} title="Verify it's on PATH">
        <Code>{`${CLI_NAME} status`}</Code>
      </Step>
    </ol>
  );
}

function WindowsSteps() {
  return (
    <ol className="flex flex-col gap-6">
      <Step n={1} title={`Unzip and move ${CLI_NAME}.exe somewhere permanent`}>
        <p className="text-xs text-[var(--ink-dim)]">
          e.g.{" "}
          <code className="text-[var(--ink)]">
            C:\Tools\{CLI_NAME}\{CLI_NAME}.exe
          </code>
        </p>
      </Step>
      <Step n={2} title="Open the environment variables editor">
        <p className="text-xs text-[var(--ink-dim)]">
          Start menu → search &ldquo;Edit the system environment
          variables&rdquo; → Environment Variables button.
        </p>
      </Step>
      <Step n={3} title='Edit "Path"'>
        <p className="text-xs text-[var(--ink-dim)]">
          Under <strong className="text-[var(--ink)]">User variables</strong>,
          select <strong className="text-[var(--ink)]">Path</strong> → Edit →
          New → paste the folder path from step 1 (
          <code className="text-[var(--ink)]">C:\Tools\{CLI_NAME}</code>, not
          the .exe itself) → OK on every dialog.
        </p>
      </Step>
      <Step n={4} title="Open a new terminal and verify">
        <p className="mb-2 text-xs text-[var(--ink-dim)]">
          PATH changes only apply to terminals opened after the edit.
        </p>
        <Code>{`${CLI_NAME} status`}</Code>
      </Step>
    </ol>
  );
}
