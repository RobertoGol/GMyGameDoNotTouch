use obfstr::obfstr;
use std::process::exit;

pub fn enforce_shield() {
    #[cfg(target_os = "windows")]
    unsafe {
        if windows_sys::Win32::System::Diagnostics::Debug::IsDebuggerPresent() != 0 {
            exit(0xDEAD); // Самоуничтожение при попытке дебага
        }
    }
}

pub fn get_node_ip(target: &str) -> String {
    match target {
        "dell" => obfstr!("192.168.6.2").to_string(),
        "termux" => obfstr!("127.0.0.1:8022").to_string(),
        _ => String::new(),
    }
}
