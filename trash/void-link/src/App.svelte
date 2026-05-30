<script>
    import { invoke } from "@tauri-apps/api/tauri";
    import TopBar from "./lib/TopBar.svelte";
    import HintBar from "./lib/HintBar.svelte";

    let password = "", totp = "", log = "READY_FOR_LINK...";

    async function handleConnect() {
        try {
            log = "ENCRYPTING...";
            const res = await invoke("start_session", { pass: password, totp: totp });
            log = res;
        } catch (e) { log = `ERROR: ${e}`; }
    }
</script>

<main class="void-link">
    <TopBar />
    
    <div class="auth-box">
        <pre class="logo">
__   _____ ___ ____  
\ \ / / _ \_ _|  _ \ 
 \ V / | | | || | | |
  \_/ \___/___|_| |_| LINK_v1.0
        </pre>
        
        <input type="password" bind:value={password} placeholder="PASS_KEY" />
        <input type="text" bind:value={totp} placeholder="2FA_CODE" maxlength="6" />
        
        <button on:click={handleConnect}>INITIALIZE</button>
        <div class="terminal">{log}</div>
    </div>

    <HintBar />
</main>

<style>
    :global(body) { margin: 0; background: #050505; overflow: hidden; }
    .void-link { height: 100vh; display: flex; flex-direction: column; color: white; font-family: 'Share Tech Mono', monospace; }
    .auth-box { margin: auto; width: 320px; padding: 30px; border: 1px solid #222; background: rgba(255,255,255,0.02); }
    input { width: 100%; background: #111; border: 1px solid #333; color: #00f0ff; padding: 10px; margin: 10px 0; outline: none; }
    button { width: 100%; background: #00f0ff; color: #000; font-weight: bold; border: none; padding: 12px; cursor: pointer; }
    button:hover { background: #fff; }
    .terminal { margin-top: 20px; font-size: 10px; color: #666; text-transform: uppercase; }
    .logo { color: #00f0ff; font-size: 10px; text-align: center; }
</style>
