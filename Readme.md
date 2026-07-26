# `kit` — Git, but it speaks human.

> Familiar Git commands, renamed to something you'll actually remember.

---

<p align="center">
  <img src="./site/public/banner.png" alt="kit banner" width="100%">
</p>

---

## Why?

Git is one of the most powerful developer tools ever built—but its vocabulary has a steep learning curve.

Commands like `checkout`, `stash`, `remote`, or `reflog` make perfect sense once you understand Git's internal model, but they're far from intuitive when you're just getting started.

I built **kit** for two reasons:

* **To understand how Git works under the hood.** Building a Git wrapper forced me to explore Git's internals, command execution, repository structure, and object model instead of treating it as a black box.
* **To experiment with data structures and CLI design.** The project became a playground for applying concepts like command parsing, lookup tables, and efficient mappings while building a polished native command-line tool.

The result is a familiar Git workflow with commands that read more like plain English.

Instead of remembering Git's terminology, you focus on what you're trying to do.

```bash
git add
```

becomes

```bash
kit stage
```

because you're **staging** changes.

```bash
git checkout feature/login
```

becomes

```bash
kit switch feature/login
```

because you're simply **switching** branches.

The goal isn't to replace Git—it's to learn from it, make it a little friendlier, and have fun building something along the way.

---

## Installation

Download the latest build.

```bash
curl -L https://kit.tier3guy.com/downloads/kit.zip -o kit.zip
```

Extract it.

```bash
unzip kit.zip
```

Move the executable.

```bash
mkdir -p ~/.local/bin
cp bin/kit ~/.local/bin/
chmod +x ~/.local/bin/kit
```

Add it to your PATH if necessary.

```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

Verify installation.

```bash
kit --help
```

---

# Quick Start

Initialize a repository.

```bash
kit new
```

Stage a file.

```bash
kit stage main.cpp
```

Save a snapshot.

```bash
kit save -m "first commit"
```

See what's changed.

```bash
kit check
```

---

# Command Reference

| Git            | Kit           | Meaning                 |
| -------------- | ------------- | ----------------------- |
| `git init`     | `kit new`     | Start a repository      |
| `git add`      | `kit stage`   | Queue changes           |
| `git commit`   | `kit save`    | Record a snapshot       |
| `git status`   | `kit check`   | Show changes            |
| `git log`      | `kit history` | Browse previous saves   |
| `git diff`     | `kit compare` | Compare changes         |
| `git branch`   | `kit fork`    | Create a branch         |
| `git checkout` | `kit switch`  | Switch branches         |
| `git merge`    | `kit join`    | Merge branches          |
| `git reset`    | `kit undo`    | Move HEAD backwards     |
| `git stash`    | `kit park`    | Save work temporarily   |
| `git remote`   | `kit link`    | Connect a remote        |
| `git clone`    | `kit copy`    | Copy a repository       |
| `git tag`      | `kit mark`    | Create a release marker |

---

# Philosophy

Git uses terminology designed around its internal model.

`kit` focuses on what the user is trying to do.

Instead of asking:

> "What's the Git command?"

You ask:

> "What am I trying to accomplish?"

Then the command is usually obvious.

| I want to...         | Command       |
| -------------------- | ------------- |
| Start a repo         | `kit new`     |
| Save work            | `kit save`    |
| See changes          | `kit check`   |
| Compare edits        | `kit compare` |
| Go to another branch | `kit switch`  |
| Merge work           | `kit join`    |

---

# Examples

Create a project.

```bash
mkdir hello
cd hello

kit new
```

Stage files.

```bash
kit stage .
```

Commit.

```bash
kit save -m "initial commit"
```

Create a branch.

```bash
kit fork feature/login
```

Switch.

```bash
kit switch feature/login
```

Merge later.

```bash
kit switch main
kit join feature/login
```

---

# Project Structure

```text
kit/
├── cli/
│   ├── src/
│   ├── build/
│   ├── bin/
│   └── Makefile
│
└── site/
```

---

# Design Goals

* Human-readable commands
* Tiny native executable
* Zero dependencies
* Familiar Git behavior
* Fast startup
* Easy to contribute

---

# FAQ

### Is this a replacement for Git?

No.

`kit` is a lightweight wrapper around Git that provides friendlier command names while relying on Git underneath.

---

# Contributing

Contributions are welcome.

Ideas include:

* Better help output
* Shell completions
* Additional aliases
* Windows improvements
* Better error messages
* More educational command descriptions

---

# License

MIT License.

---

<p align="center">
<b>Git is powerful.</b><br>
<b>Kit makes it readable.</b>
</p>
