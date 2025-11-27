BÁO CÁO CÁC THAY ĐỔI SCANNER
================================

CÂU 1: Từ khóa viết thường
- Từ khóa chỉ nhận diện khi viết thường (ví dụ: "begin" là từ khóa, "Begin" là identifier)

>> Sửa file token.c:
   - Hàm keywordEq(): Đổi từ so sánh không phân biệt hoa thường (dùng toupper) 
     sang so sánh chính xác bằng strcmp()
     Code cũ: while ((*kw != '\0') && (*string != '\0')) { if (*kw != toupper(*string)) break; ... }
     Code mới: return strcmp(kw, string) == 0;
   
   - Mảng keywords[]: Đổi tất cả từ khóa từ chữ HOA sang chữ thường
     Ví dụ: {"PROGRAM", KW_PROGRAM} → {"program", KW_PROGRAM}
            {"BEGIN", KW_BEGIN} → {"begin", KW_BEGIN}
            {"END", KW_END} → {"end", KW_END}

>> Sửa file token.c: Thay #include <ctype.h> bằng #include <string.h> để dùng strcmp()

================================

CÂU 2: Thêm 2 từ khóa "return" và "switch"

>> Sửa file token.h:
   - Tăng KEYWORDS_COUNT từ 20 lên 22
   - Thêm vào enum TokenType: KW_RETURN, KW_SWITCH (sau KW_TO)

>> Sửa file token.c:
   - Thêm vào mảng keywords[]: 
     {"return", KW_RETURN}
     {"switch", KW_SWITCH}

>> Sửa file scanner.c:
   - Hàm printToken(): Thêm 2 case mới
     case KW_RETURN: printf("KW_RETURN\n"); break;
     case KW_SWITCH: printf("KW_SWITCH\n"); break;

================================

CÂU 3: Ký hiệu [ và ] tương đương với (. và .)
- Dấu [ trả về SB_LSEL
- Dấu ] trả về SB_RSEL

>> Sửa file charcode.h:
   - Thêm vào enum CharCode (trước CHAR_UNKNOWN):
     CHAR_LBRACK    // Ký tự [
     CHAR_RBRACK    // Ký tự ]

>> Sửa file charcode.c:
   - Cập nhật mảng charCodes[] tại dòng 23 (byte 88-95):
     Thay đổi từ: CHAR_LETTER, CHAR_LETTER, CHAR_LETTER, CHAR_UNKNOWN, CHAR_UNKNOWN, CHAR_UNKNOWN, CHAR_UNKNOWN, CHAR_UNKNOWN
     Thành: CHAR_LETTER, CHAR_LETTER, CHAR_LETTER, CHAR_LBRACK, CHAR_UNKNOWN, CHAR_RBRACK, CHAR_UNKNOWN, CHAR_UNDERSCORE
     (Byte 91='[', Byte 93=']', Byte 95='_')

>> Sửa file scanner.c:
   - Hàm getToken(): Thêm 2 case mới (trước default)
     case CHAR_LBRACK:
       token = makeToken(SB_LSEL, lineNo, colNo);
       readChar();
       return token;
     
     case CHAR_RBRACK:
       token = makeToken(SB_RSEL, lineNo, colNo);
       readChar();
       return token;

================================

CÂU 4: Toán tử != và <> đều trả về SB_NEQ

>> Sửa file scanner.c:
   - Hàm getToken(), case CHAR_LT: Thêm xử lý toán tử <>
     Code cũ: 
       if (charCodes[currentChar] == CHAR_EQ) {
         readChar();
         return makeToken(SB_LE, ln, cn);
       }
       return makeToken(SB_LT, ln, cn);
     
     Code mới:
       if (currentChar != EOF) {
         if (charCodes[currentChar] == CHAR_EQ) {
           readChar();
           return makeToken(SB_LE, ln, cn);  // <=
         } else if (charCodes[currentChar] == CHAR_GT) {
           readChar();
           return makeToken(SB_NEQ, ln, cn); // <>
         }
       }
       return makeToken(SB_LT, ln, cn);

================================

CÂU 5: Thêm toán tử += và *=
- Toán tử += trả về SB_PLUS_ASSIGN
- Toán tử *= trả về SB_TIMES_ASSIGN

>> Sửa file token.h:
   - Thêm vào enum TokenType (sau SB_RSEL):
     SB_PLUS_ASSIGN
     SB_TIMES_ASSIGN

>> Sửa file scanner.c:
   - Hàm getToken(), case CHAR_PLUS: Thêm xử lý +=
     Code cũ:
       token = makeToken(SB_PLUS, lineNo, colNo);
       readChar();
       return token;
     
     Code mới:
       ln = lineNo; cn = colNo;
       readChar();
       if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
         readChar();
         return makeToken(SB_PLUS_ASSIGN, ln, cn);
       }
       return makeToken(SB_PLUS, ln, cn);
   
   - Hàm getToken(), case CHAR_TIMES: Thêm xử lý *=
     Code cũ:
       token = makeToken(SB_TIMES, lineNo, colNo);
       readChar();
       return token;
     
     Code mới:
       ln = lineNo; cn = colNo;
       readChar();
       if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
         readChar();
         return makeToken(SB_TIMES_ASSIGN, ln, cn);
       }
       return makeToken(SB_TIMES, ln, cn);
   
   - Hàm printToken(): Thêm 2 case mới
     case SB_PLUS_ASSIGN: printf("SB_PLUS_ASSIGN\n"); break;
     case SB_TIMES_ASSIGN: printf("SB_TIMES_ASSIGN\n"); break;

================================

CÂU 6: Tên có dấu gạch dưới, độ dài tối đa 9 ký tự
- Identifier có thể chứa dấu gạch dưới (_)
- Identifier có thể bắt đầu bằng gạch dưới
- Nếu identifier dài hơn 9 ký tự, chỉ lấy 9 ký tự đầu (không báo lỗi)

>> Sửa file token.h:
   - Đổi MAX_IDENT_LEN từ 15 xuống 9

>> Sửa file charcode.h:
   - Đã thêm CHAR_UNDERSCORE ở CÂU 3

>> Sửa file charcode.c:
   - Đã cập nhật charCodes[] cho ký tự '_' (byte 95) ở CÂU 3

>> Sửa file scanner.c:
   - Hàm readIdentKeyword(): 
     * Sửa điều kiện while để chấp nhận CHAR_UNDERSCORE:
       Code cũ: while (currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT))
       Code mới: while (currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT || charCodes[currentChar] == CHAR_UNDERSCORE))
     
     * Sửa xử lý khi identifier quá dài (chỉ lấy 9 ký tự đầu, không báo lỗi):
       Code cũ:
         if (count > MAX_IDENT_LEN) {
           error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
         }
         token->string[count] = '\0';
       
       Code mới:
         if (count <= MAX_IDENT_LEN) {
           token->string[count] = '\0';
         } else {
           token->string[MAX_IDENT_LEN] = '\0';
         }
   
   - Hàm getToken(): Thêm case CHAR_UNDERSCORE (cho phép identifier bắt đầu bằng _)
     case CHAR_LETTER:
     case CHAR_UNDERSCORE:
       return readIdentKeyword();

================================

CÂU 7: Chú thích dòng lệnh dạng //
- Comment bắt đầu bằng // và kết thúc ở cuối dòng

>> Sửa file scanner.c:
   - Thêm hàm skipLineComment():
     void skipLineComment() {
       readChar();  // Bỏ qua '/' thứ 2
       while (currentChar != EOF && currentChar != '\n') {
         readChar();
       }
     }
   
   - Hàm getToken(), case CHAR_SLASH: Thêm xử lý comment dòng //
     Code cũ:
       token = makeToken(SB_SLASH, lineNo, colNo);
       readChar();
       return token;
     
     Code mới:
       ln = lineNo; cn = colNo;
       readChar();
       if (currentChar != EOF && charCodes[currentChar] == CHAR_SLASH) {
         skipLineComment();
         return getToken();
       }
       return makeToken(SB_SLASH, ln, cn);

================================

TÓM TẮT CÁC FILE THAY ĐỔI:
- token.h: Thêm KW_RETURN, KW_SWITCH, SB_PLUS_ASSIGN, SB_TIMES_ASSIGN; đổi MAX_IDENT_LEN=9, KEYWORDS_COUNT=22
- token.c: Đổi keywords sang chữ thường, thêm "return", "switch"; đổi keywordEq() dùng strcmp()
- charcode.h: Thêm CHAR_LBRACK, CHAR_RBRACK, CHAR_UNDERSCORE
- charcode.c: Cập nhật mảng charCodes[] cho các ký tự [, ], _
- scanner.c: Thêm skipLineComment(); sửa readIdentKeyword(), getToken(), printToken()

