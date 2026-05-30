<script>
    let time = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
    setInterval(() => { time = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }); }, 1000);
</script>

<div class="top-bar">
    <div class="user">RUPPERT@ALPINE-DELL</div>
    <div class="time">{time}</div>
    <div class="status">● ONLINE</div>
</div>

<style>
    .top-bar { display: flex; justify-content: space-between; padding: 5px 20px; background: rgba(0,0,0,0.8); 
               border-bottom: 1px solid #00f0ff; color: #fff; font-family: monospace; font-size: 12px; }
    .status { color: #00f0ff; text-shadow: 0 0 5px #00f0ff; }
</style>