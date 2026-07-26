import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "kit — version control, rebuilt from scratch",
  description:
    "kit is a from-scratch reimplementation of Git's internals — objects, trees, refs, and all. Download the CLI and try it yourself.",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className="h-full antialiased">
      <head>
        <link rel="icon" href="/public/icon.png"></link>
      </head>
      <body className="min-h-full flex flex-col bg-[#0D0B09] text-[#F2EDE4]">
        {children}
      </body>
    </html>
  );
}
