<script>
  import { invoke } from "@tauri-apps/api/tauri";
  let isRegister = false;
  let login = "", pass = "", confirmPass = "", totp = "", status = "SYSTEM_READY";

  async function handleAction() {
    if (isRegister) {
      if (pass !== confirmPass) { status = "ERR: PASS_MISMATCH"; return; }
      status = "INITIALIZING_SECURE_VAULT...";
      // Нативный вызов регистрации в Rust
      const res = await invoke("register_user", { login, pass });
      status = res;
      isRegister = false; // Переходим на логин после успеха
    } else {
      status = "INITIATING_GHOST_LINK...";
      const res = await invoke("login_user", { login, pass, totp });
      status = res;
    }
  }
</script>

<div class="auth-frame">
  <header class="title">{isRegister ? "[ INITIALIZE_SYSTEM ]" : "[ IDENTITY_VALIDATION ]"}</header>
  
  <div class="input-stack">
    <input type="text" bind:value={login} placeholder="IDENTITY_NAME" autofocus />
    
    <input type="password" bind:value={pass} placeholder="MASTER_PASSPHRASE" />
    
    {#if isRegister}
      <input type="password" bind:value={confirmPass} placeholder="CONFIRM_PASSPHRASE" />
    {:else}
      <input type="text" bind:value={totp} placeholder="2FA_TOKEN (OPTIONAL)" maxlength="6" />
    {/if}
  </div>

  <button class="main-btn" on:click={handleAction}>
    {isRegister ? "CREATE_SECURE_VAULT" : "ESTABLISH_CONNECTION"}
  </button>

  <div class="toggle-mode" on:click={() => isRegister = !isRegister}>
    {isRegister ? "> ALREADY_INITIALIZED? LOGIN" : "> FIRST_TIME? REGISTER_NEW_NODE"}
  </div>

  <p class="console-log">> {status}</p>
</div>

<style>
  .auth-frame { width: 350px; background: rgba(0,0,0,0.9); padding: 30px; border: 1px solid #00f0ff; box-shadow: 0 0 15px #00f0ff33; }
  .title { color: #00f0ff; font-size: 12px; margin-bottom: 25px; text-align: center; }
  .input-stack input { width: 100%; background: #0a0a0a; border: none; border-bottom: 1px solid #333; color: white; padding: 12px; margin-bottom: 10px; outline: none; }
  .input-stack input:focus { border-bottom-color: #00f0ff; }
  .main-btn { width: 100%; background: #00f0ff; color: black; border: none; padding: 15px; font-weight: bold; cursor: pointer; margin-top: 15px; }
  .toggle-mode { margin-top: 15px; font-size: 9px; color: #666; cursor: pointer; text-align: center; }
  .toggle-mode:hover { color: #00f0ff; }
  .console-log { font-size: 10px; color: #444; margin-top: 20px; }
</style>
