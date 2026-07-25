import { CLI_NAME, COMMANDS } from "@/config/commands";

export default function CommandTable() {
  return (
    <div className="overflow-hidden rounded-lg border border-[var(--line)]">
      <div className="grid grid-cols-[1fr_1fr_1.4fr] gap-2 border-b border-[var(--line)] bg-[var(--bg-raised)] px-4 py-3 text-xs uppercase tracking-widest text-[var(--ink-dim)]">
        <span>git</span>
        <span>{CLI_NAME}</span>
        <span>does</span>
      </div>
      <div className="divide-y divide-[var(--line)]">
        {COMMANDS.map((c) => (
          <div
            key={c.git}
            className="grid grid-cols-[1fr_1fr_1.4fr] items-center gap-2 px-4 py-3 text-sm hover:bg-[var(--bg-raised)]/60"
          >
            <span className="text-[var(--ink-dim)] line-through decoration-[var(--line)]">
              git {c.git}
            </span>
            <span className="font-display font-semibold text-[var(--ember-2)]">
              {CLI_NAME} {c.name}
              {c.args ? (
                <span className="font-normal text-[var(--ink-dim)]"> {c.args}</span>
              ) : null}
            </span>
            <span className="text-[var(--ink-dim)]">{c.description}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
