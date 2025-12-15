// 修改函数 1：gettoken (增加 + 的处理)
// 找到 case '>': 并修改为：
case '>':
*q++ = *s;
s++;
if (*s == '>')
{ // 新增：检测 >>
    *q++ = *s;
    s++;
    ret = '+'; // 返回 + 代表追加重定向
}
else
{
    ret = '>';
}
break;

// 修改函数 2：parseredirs (处理 + 符号)
// 找到 switch(tok) 的 case '>': 附近，新增 case '+':
case '>':
case '+': // 新增这一行
cmd = redircmd(cmd, q, eq, tok == '>' ? O_WRONLY | O_CREATE | O_TRUNC : O_WRONLY | O_CREATE | O_APPEND, 1);
break;