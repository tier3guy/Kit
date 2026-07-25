import DownloadButton from "@/components/DownloadButton";
import CommandTable from "@/components/CommandTable";
import InstallGuide from "@/components/InstallGuide";
import { Github, Globe, Linkedin, Mail } from "lucide-react";

export default function Home() {
  return (
    <main className="relative flex-1">
      <div
        aria-hidden
        className="pointer-events-none absolute inset-x-0 top-0 h-[480px] opacity-[0.15]"
        style={{
          background:
            "radial-gradient(60% 60% at 50% 0%, var(--ember-1), transparent 70%)",
        }}
      />

      <section className="relative mx-auto max-w-3xl px-6 pt-20 pb-16 sm:pt-28">
        <p className="mb-4 text-xs uppercase tracking-[0.3em] text-[var(--ink-dim)]">
          a version control system, from scratch
        </p>
        <h1 className="font-display text-4xl font-extrabold leading-tight sm:text-5xl">
          git, rebuilt object
          <br />
          by <span className="ember-text">object</span>.
        </h1>
        <p className="mt-5 max-w-xl text-sm leading-relaxed text-[var(--ink-dim)] sm:text-base">
          kit re-implements the blobs, trees, commits, and refs that make Git
          work — no shortcuts, no wrapping the real thing. Built to learn how
          version control actually works, one object at a time.
        </p>

        <div className="mt-8">
          <DownloadButton />
          <p className="mt-3 text-xs text-[var(--ink-dim)]">
            latest build — extract and follow the setup steps below.
          </p>
        </div>

        <div className="mt-14 overflow-hidden rounded-lg border border-[var(--line)] bg-[var(--bg-raised)] shadow-2xl shadow-black/40">
          <div className="flex items-center gap-1.5 border-b border-[var(--line)] px-4 py-3">
            <span className="h-2.5 w-2.5 rounded-full bg-[#5a4f42]" />
            <span className="h-2.5 w-2.5 rounded-full bg-[#5a4f42]" />
            <span className="h-2.5 w-2.5 rounded-full bg-[#5a4f42]" />
            <span className="ml-3 text-xs text-[var(--ink-dim)]">
              ~/projects/hello-world
            </span>
          </div>
          <pre className="overflow-x-auto p-5 text-[13px] leading-relaxed sm:text-sm">
            <code>
              <span className="text-[var(--ink-dim)]">$ </span>
              <span className="text-[var(--ink)]">kit new</span>
              {"\n"}
              <span className="text-[var(--ink-dim)]">
                {"  "}initialized empty repository in .kit/
              </span>
              {"\n\n"}
              <span className="text-[var(--ink-dim)]">$ </span>
              <span className="text-[var(--ink)]">kit stage app.js</span>
              {"\n\n"}
              <span className="text-[var(--ink-dim)]">$ </span>
              <span className="text-[var(--ink)]">
                kit save -m &quot;first commit&quot;
              </span>
              {"\n"}
              <span className="ember-text">
                {"  "}[main a1c9f02] first commit
              </span>
              {"\n"}
              <span className="text-[var(--ink-dim)]">
                {"  "}1 file changed, 12 insertions(+)
              </span>
            </code>
          </pre>
        </div>
      </section>

      <section className="relative mx-auto max-w-3xl px-6 pb-16">
        <h2 className="font-display text-lg font-bold text-[var(--ink)]">
          add it to your PATH
        </h2>
        <p className="mt-2 text-sm text-[var(--ink-dim)]">
          so you can run it from anywhere, not just the download folder.
        </p>
        <div className="mt-6">
          <InstallGuide />
        </div>
      </section>

      <section className="relative mx-auto max-w-3xl px-6 pb-24">
        <h2 className="font-display text-lg font-bold text-[var(--ink)]">
          the command set
        </h2>
        <p className="mt-2 text-sm text-[var(--ink-dim)]">
          familiar git commands, renamed to their own thing.
        </p>
        <div className="mt-6">
          <CommandTable />
        </div>
      </section>

      <footer className="relative border-t border-[var(--line)] px-6 py-8">
        <div className="mx-auto flex max-w-3xl flex-col gap-4">
          <p className="text-xs text-center text-[var(--ink-dim)]">
            kit is a personal, from-scratch reimplementation of Git internals.
            not affiliated with the Git project.
          </p>
          <div className="flex items-center justify-center gap-4">
            <a
              href="https://github.com/tier3guy"
              target="_blank"
              rel="noopener noreferrer"
              aria-label="GitHub"
              title="GitHub"
              className="text-[var(--ink-dim)] transition-colors hover:text-[var(--ember-2)]"
            >
              <Github size={18} strokeWidth={1.75} />
            </a>
            <a
              href="https://tier3guy.com"
              target="_blank"
              rel="noopener noreferrer"
              aria-label="Website"
              title="tier3guy.com"
              className="text-[var(--ink-dim)] transition-colors hover:text-[var(--ember-2)]"
            >
              <Globe size={18} strokeWidth={1.75} />
            </a>
            <a
              href="https://www.linkedin.com/in/tier3guy/"
              target="_blank"
              rel="noopener noreferrer"
              aria-label="LinkedIn"
              title="LinkedIn"
              className="text-[var(--ink-dim)] transition-colors hover:text-[var(--ember-2)]"
            >
              <Linkedin size={18} strokeWidth={1.75} />
            </a>
            <a
              href="mailto:avinashgupta.works@gmail.com"
              aria-label="Email"
              title="Email"
              className="text-[var(--ink-dim)] transition-colors hover:text-[var(--ember-2)]"
            >
              <Mail size={18} strokeWidth={1.75} />
            </a>
          </div>
        </div>
      </footer>
    </main>
  );
}
