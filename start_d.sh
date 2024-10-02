#!/bin/bash

set -euo pipefail

# Определение цветов
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # Без цвета

# Функция для вывода сообщений с тегами и цветами
log() {
    local tag="$1"
    local message="$2"
    case "$tag" in
        INFO)
            echo -e "${GREEN}[INFO]${NC} $message"
            ;;
        WARN)
            echo -e "${YELLOW}[WARN]${NC} $message"
            ;;
        ERROR)
            echo -e "${RED}[ERROR]${NC} $message" >&2
            ;;
        DEBUG)
            echo -e "${BLUE}[DEBUG]${NC} $message"
            ;;
        *)
            echo -e "[UNKNOWN] $message"
            ;;
    esac
}

# Функция для вывода ошибок и выхода
error_exit() {
    log "ERROR" "$1"
    exit 1
}

# Функция для обработки tskv логов
process_tskv_logs() {
    while IFS= read -r line; do
        # Проверяем, начинается ли строка с 'tskv'
        if [[ "$line" == tskv* ]]; then
            # Удаляем префикс 'tskv\t'
            content="${line#tskv\t}"
            
            # Разделяем на ключи и значения
            declare -A log_entry
            IFS=$'\t' read -r -a pairs <<< "$content"
            for pair in "${pairs[@]}"; do
                key="${pair%%=*}"
                value="${pair#*=}"
                log_entry["$key"]="$value"
            done

            # Извлекаем необходимые поля
            timestamp="${log_entry[timestamp]:-}"
            level="${log_entry[level]:-}"
            module="${log_entry[module]:-}"
            text="${log_entry[text]:-}"

            # Форматируем timestamp (можно улучшить при необходимости)
            # Например, убрать "timestamp=" если уже извлечено
            # timestamp=${timestamp#timestamp=}

            # Определение цвета на основе уровня
            case "$level" in
                INFO)
                    color="$GREEN"
                    ;;
                WARNING)
                    color="$YELLOW"
                    ;;
                ERROR)
                    color="$RED"
                    ;;
                DEBUG)
                    color="$BLUE"
                    ;;
                *)
                    color="$NC"
                    ;;
            esac

            # Вывод отформатированной строки
            printf "${color}[%-7s] [%-24s] [%-30s] %s${NC}\n" "$level" "$timestamp" "$module" "$text"
        else
            # Если строка не в формате tskv, выводим как есть
            echo "$line"
        fi
    done
}

# Шаг 1: Запуск make build-debug
log "INFO" "Запуск make build-debug..."
if ! make build-debug; then
    error_exit "Команда 'make build-debug' завершилась с ошибкой."
fi
log "INFO" "Команда 'make build-debug' выполнена успешно."

# Шаг 2: Извлечение dbconnection из YAML файла
CONFIG_FILE="configs/config_vars.yaml"

if [[ ! -f "$CONFIG_FILE" ]]; then
    error_exit "Файл конфигурации $CONFIG_FILE не найден."
fi

log "INFO" "Извлечение строки подключения к базе данных из $CONFIG_FILE..."

# Используем grep и sed для извлечения значения dbconnection
DB_CONNECTION_LINE=$(grep '^dbconnection:' "$CONFIG_FILE") || error_exit "Не удалось найти строку 'dbconnection' в $CONFIG_FILE."

DB_CONNECTION=$(echo "$DB_CONNECTION_LINE" | sed -E "s/dbconnection:\s*'(.+)'/\1/") || error_exit "Не удалось извлечь значение dbconnection."

if [[ -z "$DB_CONNECTION" ]]; then
    error_exit "Значение dbconnection пусто."
fi

log "DEBUG" "Строка подключения к базе данных: $DB_CONNECTION"

# Шаг 3: Парсинг строки подключения для получения компонентов
# Пример строки подключения: postgresql://postgres:pass@localhost:5432/project_service_db_1

regex='postgresql://([^:]+):([^@]+)@([^:]+):([0-9]+)/(.+)'

if [[ $DB_CONNECTION =~ $regex ]]; then
    DB_USER="${BASH_REMATCH[1]}"
    DB_PASS="${BASH_REMATCH[2]}"
    DB_HOST="${BASH_REMATCH[3]}"
    DB_PORT="${BASH_REMATCH[4]}"
    DB_NAME="${BASH_REMATCH[5]}"
else
    error_exit "Не удалось разобрать строку подключения dbconnection."
fi

log "DEBUG" "Пользователь БД: $DB_USER"
log "DEBUG" "Хост БД: $DB_HOST"
log "DEBUG" "Порт БД: $DB_PORT"
log "DEBUG" "Имя БД: $DB_NAME"

# Шаг 4: Проверка существования базы данных
log "INFO" "Проверка существования базы данных $DB_NAME..."

DB_EXISTS=$(PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -U "$DB_USER" -p "$DB_PORT" -tAc "SELECT 1 FROM pg_database WHERE datname='$DB_NAME'" 2>/dev/null) || error_exit "Не удалось подключиться к PostgreSQL."

if [[ "$DB_EXISTS" != "1" ]]; then
    log "WARN" "База данных $DB_NAME не существует. Создание..."
    PGPASSWORD="$DB_PASS" createdb -h "$DB_HOST" -U "$DB_USER" -p "$DB_PORT" "$DB_NAME" || error_exit "Не удалось создать базу данных $DB_NAME."
    log "INFO" "База данных $DB_NAME успешно создана."
else
    log "INFO" "База данных $DB_NAME уже существует."
fi

# Шаг 5: Выполнение SQL скрипта
# Извлекаем суффикс после 'project_service_'
PREFIX="project_service_"
if [[ "$DB_NAME" == "$PREFIX"* ]]; then
    DB_SUFFIX="${DB_NAME#$PREFIX}"
else
    error_exit "Имя базы данных не начинается с $PREFIX."
fi

SQL_FILE="postgresql/schemas/${DB_SUFFIX}.sql"

if [[ ! -f "$SQL_FILE" ]]; then
    error_exit "SQL файл $SQL_FILE не найден."
fi

log "INFO" "Выполнение SQL скрипта $SQL_FILE на базе данных $DB_NAME..."
PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -U "$DB_USER" -p "$DB_PORT" -d "$DB_NAME" -f "$SQL_FILE" || error_exit "Не удалось выполнить SQL скрипт."
log "INFO" "SQL скрипт $SQL_FILE успешно выполнен."

# Шаг 6: Запуск project_service с обработкой логов
EXECUTABLE="./build_debug/project_service"
CONFIG_STATIC="configs/static_config.yaml"
CONFIG_VARS="configs/config_vars.yaml"

if [[ ! -x "$EXECUTABLE" ]]; then
    error_exit "Исполняемый файл $EXECUTABLE не найден или не имеет прав на исполнение."
fi

log "INFO" "Запуск $EXECUTABLE с конфигурационными файлами..."

# Запуск исполняемого файла и обработка его вывода через process_tskv_logs
# Используем subshell для обработки stdout и stderr отдельно
{
    "$EXECUTABLE" -c "$CONFIG_STATIC" --config_vars "$CONFIG_VARS" 2>&1
} | process_tskv_logs

if [[ $? -eq 0 ]]; then
    log "INFO" "Проект успешно запущен."
else
    error_exit "Запуск $EXECUTABLE завершился с ошибкой."
fi

log "INFO" "Скрипт выполнен успешно."
