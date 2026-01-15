# Playbook: No-Contact Diagnostics from Git Status and Recent Commits

*Status: Draft*

## Objective
Diagnose likely causes of a problem using only `git status`, diffs, and recent commit history, without modifying any files.

## Prerequisites
*   Git installed and available in PowerShell.
*   Local repository access.
*   Read-only intent: no file edits, no restores, no checkouts.

## Step-by-Step Instructions
1.  **Snapshot Working Tree State**
    *   Command:
        ```powershell
        git status -sb
        ```
    *   Expected output: shows current branch and a list of modified/untracked files.
    *   If it fails: confirm you are in the repo root, then retry.

2.  **Summarize Local Changes**
    *   Commands:
        ```powershell
        git diff --stat
        git diff
        ```
    *   Expected output: a per-file change summary and full diffs for uncommitted changes.
    *   If output is too large: narrow to a file or folder:
        ```powershell
        git diff -- firmware/web/index.html
        ```

3.  **Check Recent Commits (If Working Tree Is Clean or Ambiguous)**
    *   Commands:
        ```powershell
        git log -n 5 --oneline
        git show -1 --stat
        ```
    *   Expected output: recent commits with file lists.
    *   If you need a specific file change:
        ```powershell
        git show -1 -- firmware/web/index.html
        ```

4.  **Correlate Errors with Diffs**
    *   Compare reported error locations (e.g., JS `SyntaxError`) with diffs in `firmware/web/index.html` or embedded UI sources.
    *   Pay special attention to:
        -   Unclosed tags or mismatched braces.
        -   Stray commas, duplicated fragments, or partially inserted handlers.
        -   Auto-generated files (e.g., `AllSeeingEye/src/WebStatic.h`) that might reflect stale or broken HTML/JS.

5.  **Record a Read-Only Diagnosis**
    *   Provide a concise summary of:
        -   Files changed.
        -   The exact diff region likely causing the issue.
        -   Whether the change is local (uncommitted) or comes from a recent commit.

## Verification
A successful diagnostic report includes:
*   The `git status -sb` output (or a paraphrase).
*   The specific file(s) implicated.
*   A clear explanation of why the error maps to the diff.

## Notes
*   This playbook is intentionally read-only and does not authorize edits.
*   If a fix is needed, create or reference a separate playbook for patching.
