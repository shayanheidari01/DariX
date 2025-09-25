# DariX Enhanced Error Tracking System

DariX حالا دارای سیستم ردیابی خطاهای پیشرفته‌ای است که جزئیات کاملی از خطاها ارائه می‌دهد.

## ✨ ویژگی‌های جدید

### 1. اطلاعات موقعیت دقیق
- شماره خط و ستون خطا
- نام فایل منبع
- Context کد اطراف خطا

### 2. انواع خطاهای مختلف
- **RuntimeError**: خطاهای زمان اجرا
- **SyntaxError**: خطاهای نحوی
- **TypeError**: خطاهای نوع داده
- **NameError**: متغیرهای تعریف نشده
- **ZeroDivisionError**: تقسیم بر صفر

### 3. Stack Trace کامل
- ردیابی کامل call stack
- نمایش مسیر فراخوانی توابع
- Context هر سطح از فراخوانی

### 4. پیشنهادات مفید
- راهنمایی برای رفع خطا
- پیشنهادات بر اساس نوع خطا
- نکات برنامه‌نویسی

## 🔧 استفاده

### خطاهای ساده
```dax
var result = 10 / 0;  // ZeroDivisionError با جزئیات کامل
```

### خطاهای تابع با Stack Trace
```dax
func level3() {
    return undefined_variable;  // NameError با stack trace
}

func level2() {
    return level3();
}

func level1() {
    return level2();
}

level1();  // نمایش کامل مسیر فراخوانی
```

### مدیریت خطا با Try-Catch
```dax
try {
    var result = risky_operation();
} catch (e) {
    print("خطا رخ داد:", e);
    // e شامل تمام جزئیات خطا است
}
```

## 📊 ساختار خطا

هر خطا شامل اطلاعات زیر است:

```
ErrorType at filename:line:column: message

Suggestion: helpful tip for fixing the error

Stack trace:
  at function_name (filename:line:column)
    context_code_line
  at another_function (filename:line:column)
    context_code_line
```

## 🎯 مثال کامل

```dax
class Calculator {
    func divide(self, a, b) {
        if (b == 0) {
            throw "Cannot divide by zero in Calculator";
        }
        return a / b;
    }
}

func test_calculator() {
    var calc = Calculator();
    return calc.divide(10, 0);  // خطا با context کلاس
}

test_calculator();
```

خروجی:
```
RuntimeError at example.dax:4:13: Cannot divide by zero in Calculator

Suggestion: Check your logic and ensure all operations are valid.

Stack trace:
  at Calculator.divide (example.dax:4:13)
    throw "Cannot divide by zero in Calculator";
  at test_calculator (example.dax:11:12)
    return calc.divide(10, 0);
  at <main> (example.dax:14:1)
    test_calculator();
```

## 🚀 مزایا

1. **دیباگ آسان‌تر**: پیدا کردن سریع محل خطا
2. **اطلاعات کامل**: جزئیات کافی برای رفع مشکل
3. **راهنمایی مفید**: پیشنهادات عملی برای حل مشکل
4. **Stack Trace**: درک بهتر جریان اجرای برنامه
5. **سازگاری**: کار با تمام ویژگی‌های DariX

سیستم خطاهای بهبود یافته DariX تجربه توسعه را بهبود می‌بخشد و کمک می‌کند تا خطاها سریع‌تر و دقیق‌تر شناسایی و رفع شوند.
