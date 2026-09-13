# Connect Kaggle GPUs to VS Code via Cloudflare Tunnel

A complete guide to using Kaggle's free GPUs (T4×2 / P100) from VS Code with full terminal access, persistent extensions, and a stable hostname that never changes between sessions.

## Architecture

```
VS Code (local) ──ProxyCommand──▶ cloudflared (local)
                                        │
                                   Cloudflare Edge
                                   (kaggle.yourdomain.com)
                                        │
                               cloudflared (Kaggle) ──▶ sshd (localhost:22)
```

Your local machine never talks directly to Kaggle. Traffic flows through a Cloudflare Tunnel, which gives you a fixed hostname. No ports to track, no SSH config to edit between sessions.

## Prerequisites

- A domain registered on or added to Cloudflare
- A free [Cloudflare Zero Trust](https://one.dash.cloudflare.com/) account
- VS Code installed locally
- A Kaggle account with phone verification (required for GPU + internet access)

---

## Part 1 — One-Time: Cloudflare Dashboard Setup

1. Go to [Cloudflare Zero Trust Dashboard](https://one.dash.cloudflare.com/) → **Networks** → **Tunnels** → **Create a tunnel**.
2. Select **Cloudflared** as the tunnel type.
3. Name it something like `kaggle-gpu`.
4. On the "Install connector" step, you'll see a command containing a long token (starts with `eyJ...`). **Copy and save that token securely** (e.g. a password manager). Don't run the command — you'll run it on Kaggle later. Click **Next**.
5. On the **Public Hostnames** tab, configure:
   - **Subdomain:** `kaggle` (or anything you like)
   - **Domain:** select your domain from the dropdown
   - **Type:** `SSH`
   - **URL:** `localhost:22`
6. **Save** the tunnel.

You now have a stable hostname (e.g. `kaggle.yourdomain.com`) that will never change.

---

## Part 2 — One-Time: Local Machine Setup

### Install cloudflared

**macOS:**

```bash
brew install cloudflared
```

**Windows:**

```bash
winget install --id Cloudflare.cloudflared
```

**Linux:**

Download the latest release from [cloudflare/cloudflared releases](https://github.com/cloudflare/cloudflared/releases).

### Configure SSH

Add this to `~/.ssh/config`:

```
Host kaggle.yourdomain.com
    HostName kaggle.yourdomain.com
    User root
    ProxyCommand cloudflared access ssh --hostname %h
    StrictHostKeyChecking no
    UserKnownHostsFile /dev/null
```

Replace `kaggle.yourdomain.com` with whatever hostname you set in Part 1.

> **Why `StrictHostKeyChecking no`?** Every new Kaggle session is a fresh container with new SSH host keys. Without this, you'd get a "Remote host key has changed" error every time and have to manually run `ssh-keygen -R kaggle.yourdomain.com`.

### Configure VS Code

1. Install the **Remote - SSH** extension.
2. Press `Ctrl+Shift+P` → **"Open User Settings (JSON)"**.
3. Add auto-install extensions for remote hosts:

```json
{
    "remote.SSH.defaultExtensions": [
        "anthropic.claude-code",
        "ms-python.python",
        "ms-toolsai.jupyter"
    ]
}
```

This ensures Claude Code, Python, and Jupyter extensions are automatically installed on every new Kaggle session without manual action.

---

## Part 3 — One-Time: Kaggle Secrets

Store sensitive values as Kaggle Secrets so they aren't exposed in your notebook code.

1. Open any Kaggle notebook.
2. Go to **Add-ons** → **Secrets**.
3. Add a secret named `CF_TUNNEL_TOKEN` with the tunnel token from Part 1.

---

## Part 4 — Each Session: Kaggle Notebook Setup

Create a notebook (or reuse one) with the following session settings in the right sidebar:

- **Accelerator:** GPU T4×2 or GPU P100
- **Persistence:** Files only
- **Internet:** On

> **Persistence: Files only** means files in `/kaggle/working/` survive session restarts (up to ~20 GB), but Python variables in memory don't. This is more reliable than "Variables and Files" for SSH workflows.

### The Setup Cell

Paste this into a single cell and run it at the start of every session:

```python
# ──────────────────────────────────────────────
# 1. SSH Server
# ──────────────────────────────────────────────
SSH_PASSWORD = "your-password-here"  # change this

!echo "root:{SSH_PASSWORD}" | chpasswd
!apt-get update -qq && apt-get install -qq -y openssh-server > /dev/null
!mkdir -p /var/run/sshd
!echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
!echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config
!service ssh start

# ──────────────────────────────────────────────
# 2. Fix PATH for nvidia-smi and nvcc
# ──────────────────────────────────────────────
# The SSH shell doesn't inherit the notebook kernel's environment.
# nvidia-smi lives at /opt/bin/, nvcc at /usr/local/cuda/bin/,
# and the NVIDIA driver libs are at /usr/local/nvidia/lib64/.
!echo 'export PATH=/opt/bin:/usr/local/cuda/bin:$PATH' >> /root/.bashrc
!echo 'export LD_LIBRARY_PATH=/usr/local/nvidia/lib64:/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> /root/.bashrc

# ──────────────────────────────────────────────
# 3. Persist Claude Code OAuth credentials
# ──────────────────────────────────────────────
# Symlink ~/.claude to persistent storage so you only
# sign in once. After the first session, credentials
# survive restarts because /kaggle/working/ is persisted.
!mkdir -p /kaggle/working/.claude
!ln -sf /kaggle/working/.claude /root/.claude

# ──────────────────────────────────────────────
# 4. Cloudflare Tunnel
# ──────────────────────────────────────────────
!wget -q https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64.deb
!dpkg -i cloudflared-linux-amd64.deb

from kaggle_secrets import UserSecretsClient
TUNNEL_TOKEN = UserSecretsClient().get_secret("CF_TUNNEL_TOKEN")
```

### The Tunnel Cell

Paste this in the next cell. It will keep running for the entire session:

```python
import subprocess

proc = subprocess.Popen(
    ["cloudflared", "tunnel", "run", "--token", TUNNEL_TOKEN],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE
)
print(f"Tunnel running (PID: {proc.pid})")
print("Connect from VS Code now.")
```

---

## Part 5 — Connect from VS Code

1. Press `Ctrl+Shift+P` → **Remote-SSH: Connect to Host…**
2. Select `kaggle.yourdomain.com`.
3. Enter your SSH password when prompted.
4. Once connected, open the folder `/kaggle` to see your workspace.

You now have:

- **Full terminal** (`Ctrl+J` / `Ctrl+\``)
- **GPU access** — run `nvidia-smi` and `nvcc --version` to verify
- **File explorer** for the Kaggle filesystem
- **Claude Code** and other extensions auto-installed

---

## Kaggle Filesystem Reference

| Path | Description | Writable | Persists |
|---|---|---|---|
| `/kaggle/working/` | Your workspace (~20 GB) | ✅ | ✅ |
| `/kaggle/input/` | Attached datasets | ❌ | N/A |
| `/kaggle/tmp/` | Temporary storage | ✅ | ❌ |
| `/root/` | Home directory | ✅ | ❌ |

Save anything you want to keep across sessions to `/kaggle/working/`.

---

## Troubleshooting

### `nvidia-smi: command not found`

The SSH shell has a different PATH than the notebook kernel. The setup cell above fixes this, but if you need to debug manually:

```bash
find / -name "nvidia-smi" 2>/dev/null
# Typically found at /opt/bin/nvidia-smi
export PATH=/opt/bin:$PATH
```

### `NVIDIA-SMI couldn't find libnvidia-ml.so`

The NVIDIA driver shared library isn't in the library search path:

```bash
find / -name "libnvidia-ml.so*" 2>/dev/null
# Typically found at /usr/local/nvidia/lib64/
export LD_LIBRARY_PATH=/usr/local/nvidia/lib64:$LD_LIBRARY_PATH
```

### `nvcc: command not found`

CUDA toolkit is installed but not in PATH:

```bash
ls /usr/local/cuda/bin/nvcc
# If it exists:
export PATH=/usr/local/cuda/bin:$PATH
```

### `Remote host key has changed`

Every new Kaggle session generates new SSH host keys. Fix:

```bash
ssh-keygen -R kaggle.yourdomain.com
```

Or add `StrictHostKeyChecking no` and `UserKnownHostsFile /dev/null` to your SSH config (already included in the config above).

### VS Code: `Permission denied` on `.vscode-server`

This happens if you symlinked `~/.vscode-server` to `/kaggle/working/`. The working directory is mounted with `noexec`, so VS Code's server binaries can't execute. **Do not symlink `.vscode-server`**. Use `remote.SSH.defaultExtensions` in your local VS Code settings to auto-install extensions instead.

### Claude Code asks to sign in every session

The OAuth tokens are stored on the remote machine and lost when the container is wiped. The setup cell above symlinks `~/.claude` to `/kaggle/working/.claude` to persist them. If you still need to re-auth, make sure the symlink exists before you connect from VS Code.

### GPU hardware not visible at all

Verify the GPU devices exist:

```bash
ls /dev/nvidia*
```

If nothing shows up, the session doesn't have GPU enabled. Go to the Kaggle notebook sidebar → Session options → Accelerator → select a GPU.

---

## Resource Limits

| Resource | Limit |
|---|---|
| GPU time | 30 hours/week |
| Session duration | 12 hours max |
| Working storage | ~20 GB |
| Input storage | Unlimited (read-only) |

---

## Alternatives to Cloudflare Tunnel

| Method | Stable Address | Free | Setup Complexity |
|---|---|---|---|
| **Cloudflare Tunnel** | ✅ Fixed hostname | ✅ | Medium |
| **Tailscale** | ✅ Fixed tailnet IP | ✅ | Medium |
| **ngrok (free)** | ❌ Changes every session | ✅ | Low |
| **ngrok (paid)** | ✅ Reserved TCP address | ❌ ~$8/mo | Low |
| **zrok** | ✅ Reserved share | ✅ | Medium |
