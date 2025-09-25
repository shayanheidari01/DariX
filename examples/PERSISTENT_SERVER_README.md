# 🌐 DariX Persistent Web Server

یک سرور HTTP کامل و مداوم که با زبان برنامه‌نویسی DariX پیاده‌سازی شده است.

## 🚀 راه‌اندازی سریع

```bash
# اجرای سرور مداوم
./darix.exe run examples/persistent_web_server.dax
```

سرور روی پورت 3000 راه‌اندازی می‌شود و تا زمانی که دستی متوقف نشود، به صورت مداوم اجرا می‌شود.

## 🌟 ویژگی‌های کلیدی

### ✅ سرور همیشه فعال
- **مداوم بودن**: سرور تا زمان توقف دستی اجرا می‌شود
- **خودکار نیست**: بر خلاف demo، خودکار متوقف نمی‌شود
- **مقاوم**: مدیریت خطا و بازیابی خودکار

### 📊 داشبورد تعاملی
- **آمار زنده**: شمارش درخواست‌ها و زمان فعالیت
- **به‌روزرسانی خودکار**: هر 30 ثانیه refresh می‌شود
- **رابط کاربری مدرن**: طراحی responsive و زیبا
- **نمایش وضعیت**: اطلاعات کامل سرور

### 🛣️ API های کامل
- **RESTful Design**: طراحی استاندارد REST API
- **پاسخ JSON**: تمام API ها JSON برمی‌گردانند
- **مدیریت پارامتر**: پردازش query parameters
- **Header Processing**: دسترسی به request headers

## 🔗 Endpoint های موجود

### 🏠 Dashboard
```
GET /
```
داشبورد تعاملی با آمار زنده و لینک‌های API

### 🌐 API Endpoints

#### 1. سلام API
```
GET /api/hello
```
**پاسخ نمونه:**
```json
{
  "message": "Hello from DariX Persistent Server!",
  "timestamp": 1640995200,
  "server": {
    "name": "DariX Persistent HTTP Server",
    "version": "1.0.0",
    "uptime_seconds": 3600,
    "total_requests": 42
  },
  "status": "running_continuously"
}
```

#### 2. زمان سرور
```
GET /api/time
```
**پاسخ نمونه:**
```json
{
  "current_time": 1640995200,
  "unix_timestamp": 1640995200,
  "server_start_time": 1640991600,
  "uptime_seconds": 3600,
  "timezone": "Server Local Time"
}
```

#### 3. اطلاعات کاربر
```
GET /api/user?name=John&age=25&email=john@example.com
```
**پاسخ نمونه:**
```json
{
  "user": {
    "name": "John",
    "age": "25",
    "email": "john@example.com",
    "id": "user_1640995200",
    "session_id": "sess_42"
  },
  "request_info": {
    "method": "GET",
    "endpoint": "/api/user",
    "timestamp": 1640995200,
    "request_number": 42
  }
}
```

#### 4. وضعیت سرور
```
GET /api/status
```
**پاسخ نمونه:**
```json
{
  "server": {
    "name": "DariX Persistent HTTP Server",
    "status": "running",
    "port": 3000,
    "uptime": {
      "seconds": 3600,
      "minutes": 60,
      "hours": 1,
      "formatted": "1h 0m"
    }
  },
  "statistics": {
    "total_requests": 42,
    "requests_per_minute": 0.7,
    "average_response_time": "< 1ms"
  }
}
```

#### 5. بررسی سلامت
```
GET /api/health
```
**پاسخ نمونه:**
```json
{
  "status": "healthy",
  "uptime_seconds": 3600,
  "total_requests": 42,
  "checks": {
    "server_running": true,
    "endpoints_responsive": true,
    "memory_usage": "normal"
  }
}
```

#### 6. ارسال داده
```
POST /api/data
Content-Type: application/json

{
  "message": "Hello Server",
  "data": {"key": "value"}
}
```

## 🧪 تست با curl

### تست‌های پایه
```bash
# تست سلام API
curl http://localhost:3000/api/hello

# تست زمان سرور
curl http://localhost:3000/api/time

# تست اطلاعات کاربر
curl "http://localhost:3000/api/user?name=Ahmad&age=30"

# تست وضعیت سرور
curl http://localhost:3000/api/status

# تست سلامت سرور
curl http://localhost:3000/api/health
```

### تست POST
```bash
# ارسال داده JSON
curl -X POST http://localhost:3000/api/data \
  -H "Content-Type: application/json" \
  -d '{"message": "Hello from curl", "test": true}'

# ارسال داده متنی
curl -X POST http://localhost:3000/api/data \
  -H "Content-Type: text/plain" \
  -d "Simple text data"
```

## 📊 نظارت و آمار

### آمار زنده
- **شمارش درخواست‌ها**: تعداد کل درخواست‌های دریافتی
- **زمان فعالیت**: مدت زمان اجرای سرور
- **درخواست در دقیقه**: میانگین درخواست‌ها
- **وضعیت سلامت**: بررسی عملکرد سرور

### لاگ‌ها
سرور تمام درخواست‌ها را با جزئیات زیر لاگ می‌کند:
```
📝 [1640995200] GET /api/hello - Mozilla/5.0...
⚡ Request processed in 0ms
```

## ⚙️ تنظیمات سرور

### پیکربندی
- **پورت**: 3000 (قابل تغییر در کد)
- **Timeout**: 60 ثانیه
- **Auto-refresh**: 30 ثانیه
- **Logging**: فعال برای تمام درخواست‌ها

### Middleware ها
1. **Logging Middleware**: ثبت تمام درخواست‌ها
2. **Performance Middleware**: اندازه‌گیری زمان پردازش

## 🛑 توقف سرور

### روش‌های توقف
1. **Ctrl+C**: توقف graceful از terminal
2. **Process Kill**: `kill -TERM <process_id>`
3. **System Shutdown**: توقف خودکار با سیستم

### توقف ایمن
سرور از graceful shutdown پشتیبانی می‌کند:
- درخواست‌های در حال پردازش تکمیل می‌شوند
- اتصالات به صورت ایمن بسته می‌شوند
- منابع آزاد می‌شوند

## 🔧 عیب‌یابی

### مشکلات رایج

#### پورت در حال استفاده
```
Error: Port 3000 already in use
```
**راه‌حل:**
1. پورت را در کد تغییر دهید
2. یا process استفاده‌کننده از پورت را متوقف کنید

#### مجوزهای شبکه
```
Error: Permission denied
```
**راه‌حل:**
1. برنامه را با مجوز administrator اجرا کنید
2. یا از پورت بالاتر از 1024 استفاده کنید

### بررسی وضعیت
```bash
# بررسی پورت
netstat -an | grep :3000

# تست اتصال
telnet localhost 3000

# بررسی سلامت
curl http://localhost:3000/api/health
```

## 🚀 توسعه و سفارشی‌سازی

### اضافه کردن Endpoint جدید
```dax
func my_api_handler(request, response) {
    var data = {"custom": "response"};
    return response_json(response, data);
}

server_route(server_id, "GET", "/api/custom", my_api_handler);
```

### اضافه کردن Middleware
```dax
func custom_middleware(request, response, next) {
    // پردازش قبل از درخواست
    var result = next();
    // پردازش بعد از درخواست
    return result;
}

server_middleware(server_id, custom_middleware);
```

### سرو فایل‌های استاتیک
```dax
server_static(server_id, "/static/", "./public/");
```

## 📈 بهینه‌سازی عملکرد

### توصیه‌ها
1. **Caching**: پیاده‌سازی cache برای پاسخ‌های تکراری
2. **Connection Pooling**: مدیریت اتصالات پایگاه داده
3. **Load Balancing**: توزیع بار برای ترافیک بالا
4. **Monitoring**: نظارت مداوم بر عملکرد

### محدودیت‌ها
- **Memory Usage**: نظارت بر مصرف حافظه
- **Request Rate**: محدودیت تعداد درخواست
- **File Descriptors**: مدیریت اتصالات همزمان

## 🌟 ویژگی‌های پیشرفته

### امکانات آینده
- [ ] Authentication & Authorization
- [ ] Database Integration
- [ ] WebSocket Support
- [ ] Template Engine
- [ ] File Upload Handling
- [ ] Rate Limiting
- [ ] SSL/TLS Support

## 📞 پشتیبانی

### منابع
- **مستندات DariX**: `docs/NATIVE_LIBRARIES.md`
- **نمونه‌های کد**: `examples/` directory
- **تست‌ها**: `examples/*_test.dax`

### گزارش مشکل
برای گزارش مشکلات یا پیشنهادات، از issue tracker پروژه استفاده کنید.

---

🎉 **سرور DariX آماده و در حال اجرا است!**

Visit: http://localhost:3000
