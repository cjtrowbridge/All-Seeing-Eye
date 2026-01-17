# Playbook: How to Commit and Push Changes

*Status: Draft*

## Objective
Provide a repeatable workflow to summarize staged changes, propose a commit message, and push to `origin` after user approval, without assuming untracked files should be added.

## Prerequisites
*   Git installed and available in PowerShell.
*   Changes staged (`git add ...`).
*   Access to the remote `origin`.

## Step-by-Step Instructions

1.  **Check Repository Status**
    *   Command: `git status -sb`
    *   Expected: Shows staged files under `Changes to be committed`, and any untracked files under `??`.
    *   If nothing staged: run `git add <files>` and re-check.

2.  **Handle Untracked Files (Never Assume They Should Be Added)**
    *   If untracked files exist, list them explicitly and ask the user to choose one of:
        *   **Add to Git Ignore** add untracked files to `.gitignore`, then re-check status.
        *   **Add** specific untracked files to staging, then re-check status.
    *   Do not add new files without explicit user instruction.

3.  **Review Staged Diff with `dump_staged.ps1`**
    *   Command (from repo root): `./dump_staged.ps1 > staged_dump.txt`
    *   Expected: `staged_dump.txt` contains the full staged file contents.
    *   If the script warns “No staged changes found”, return to Step 1.

4.  **Summarize Changes**
    *   Read `staged_dump.txt` and produce:
        *   A concise bullet list of changes since the last commit.
        *   A single-sentence commit message suggestion (imperative mood).

5.  **Request Approval**
    *   Ask the user to approve the summary and commit message.
    *   Confirm how to handle any untracked files (ignore, add specific files, or stop).
    *   Do **not** commit until explicit approval is given.

6.  **Commit After Approval**
    *   Command: `git commit -m "<approved message>"`
    *   Expected: Commit created with the approved message.
    *   If commit fails: re-check staged files and resolve any errors.

7.  **Push to Origin**
    *   Command: `git push origin HEAD`
    *   Expected: Remote updated with the new commit.
    *   If push fails: capture the error output and report it.

## Verification
*   `git log -1 --oneline` shows the new commit.
*   `git status -sb` is clean.
*   `git push origin HEAD` succeeds.
