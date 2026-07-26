# `kit` — Git, but it speaks human.

> Familiar Git commands, renamed to something you'll actually remember.

---

<p align="center">
  <img src="./site/public/banner.png" alt="kit banner" width="100%">
</p>

---

## Why?

Git is incredibly powerful.

Its naming... isn't.

When you're learning version control, commands like `checkout`, `stash`, `reflog`, or `remote` don't explain *what they actually do*.

**kit** keeps Git's power while replacing command names with words that describe the action.

Instead of memorizing Git jargon, you read commands that make sense.

```bash
git add
```

becomes

```bash
kit stage
```

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

### Does it change my repositories?

No.

Repositories remain standard Git repositories.

You can freely switch between `git` and `kit`.

---

### Can I still use Git?

Absolutely.

These work interchangeably.

```bash
kit save -m "update"

git status

kit history

git log
```

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
