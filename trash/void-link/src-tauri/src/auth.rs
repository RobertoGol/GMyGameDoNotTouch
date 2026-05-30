use keyring::Entry;
use tauri::command;
use crate::security;
use zeroize::Zeroize;

#[command]
pub async fn register_user(login: String, mut pass: String) -> Result<String, String> {
    security::enforce_shield(); // Проверка на взлом перед регистрацией

    // Создаем запись пользователя в системном хранилище
    let entry = Entry::new("AEGIS_VAULT", &login).map_err(|e| e.to_string())?;
    
    // В реальности здесь мы бы создали соль и хешировали пароль перед сохранением
    entry.set_password(&pass).map_err(|e| e.to_string())?;
    
    pass.zeroize(); // Гарантированно затираем пароль в RAM
    
    Ok("VAULT_INITIALIZED_SUCCESSFULLY".to_string())
}

#[command]
pub async fn login_user(login: String, mut pass: String, totp: String) -> Result<String, String> {
    let entry = Entry::new("AEGIS_VAULT", &login).map_err(|e| e.to_string())?;
    let saved_pass = entry.get_password().map_err(|_| "IDENTITY_NOT_FOUND")?;

    if pass == saved_pass {
        pass.zeroize();
        // Здесь начинается логика загрузки Payload
        Ok("GHOST_LINK_ACTIVE".to_string())
    } else {
        Err("ACCESS_DENIED: INVALID_CREDENTIALS".to_string())
    }
}
