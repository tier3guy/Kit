export default function DownloadButton() {
  return (
    <a
      href="/downloads/kit.zip"
      download
      className="group relative inline-flex items-center gap-3 overflow-hidden rounded-md px-6 py-3 font-display text-sm font-bold text-[#0d0b09] transition-transform hover:-translate-y-0.5"
    >
      <span className="ember-bg absolute inset-0" />
      <span className="relative">download kit.zip</span>
      <span className="relative text-base transition-transform group-hover:translate-x-0.5">
        &darr;
      </span>
    </a>
  );
}
