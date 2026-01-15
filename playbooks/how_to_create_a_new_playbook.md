# Playbook: How to Create a New Agent Playbook

*Status: Stable*

This playbook outlines the standard procedure for an AI Agent to create a new operational playbook for the All Seeing Eye project. Playbooks are essential for standardizing complex tasks, troubleshooting, and development workflows.

## 1. Prerequisites & Context Gathering

**CRITICAL STEP**: Before attempting to write a new playbook, you must establish a complete mental model of the system. Do not skip this.

1.  **Read the Root Documentation**:
    *   `README.md`: Understand the project's physical form, high-level architecture (SDR vs. Eye nodes), and goals.
    *   `AGENTS.md`: Understand the organizational structure, API standards, and the role of agents.
2.  **Read Component Documentation**:
    *   `firmware/README.md`: Understand the C++ implementation, networking logic, and current capabilities.
    *   `api/README.md`: Understand how the desktop client and LLMs interact with the cluster.
3.  **Verify Current State**:
    *   Check `playbooks/` to ensure a similar playbook does not already exist.

## 2. When to Create a Playbook

Create a new playbook when:
*   **Complexity**: A task involves more than 3 distinct steps or spans multiple domains (e.g., Firmware + Python + Documentation).
*   **Repetition**: The user asks for the same multi-step operation frequently.
*   **Troubleshooting**: You successfully solve a difficult error (e.g., "Build failed due to missing library") and want to document the fix for future agents.
*   **Workflow**: A new feature is added that requires a specific deployment or testing sequence.

## 3. Drafting the Playbook

### Filename Convention
*   Use **verbose, descriptive filenames** using snake_case.
*   The filename should be a sentence fragment that answers "What is this for?".
*   *Bad*: `deploy.md`, `fix_error.md`
*   *Good*: `how_to_deploy_firmware_updates.md`, `troubleshooting_offline_nodes.md`

### File Structure
Start with the following template:

```markdown
# Playbook: [Title of the Task]

*Status: [Draft | Stable | Deprecated]*

## Objective
A 1-sentence summary of what this playbook achieves.

## Prerequisites
*   Tools required (e.g., PowerShell, Arduino CLI).
*   Access required (e.g., Local Network, USB Cable).

## Step-by-Step Instructions
1.  **Step Name**: 
    *   Command to run.
    *   Expected output.
    *   What to do if it fails.

## Verification
How to confirm the task was successful.
```

## 4. Writing Guidelines

*   **Be Specific**: Do not say "Run the script." Say "Run `.\firmware\upload_ota.ps1` from the project root."
*   **Anticipate Failure**: If a step is prone to error (like network timeouts), provide a specific remediation sub-step.
*   **Code-First**: Where possible, reference specific scripts in the repo rather than writing long manual terminal commands.
*   **Idempotency**: Playbooks should ideally be repeatable without breaking the system.

## 5. Finalizing

1.  Save the file to `playbooks/`.
2.  (Optional) If this is a major workflow, mention it in `AGENTS.md` under the "Agent Playbooks" section.
