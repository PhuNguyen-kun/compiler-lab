/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

 #include <stdio.h>
 #include <stdlib.h>
 
 #include "reader.h"
 #include "charcode.h"
 #include "token.h"
 #include "error.h"
 
 
 extern int lineNo;
 extern int colNo;
 extern int currentChar;
 
 extern CharCode charCodes[];
 
 /***************************************************************/
 
 void skipBlank() {
   while (currentChar != EOF && charCodes[currentChar] == CHAR_SPACE) {
     readChar();
   }
 }
 
 void skipComment(int ln, int cn) {
   // Khi vào hàm này, currentChar = '*' (sau '(')
   // Mục tiêu: đọc cho đến khi gặp '*)'
   // ln, cn là vị trí của '(' mở đầu comment
   
   readChar(); // Bỏ qua '*' trong '(*'
   
   // Đọc cho đến khi gặp '*)'
   while (currentChar != EOF) {
     if (charCodes[currentChar] == CHAR_TIMES) {
       // Gặp '*', kiểm tra ký tự tiếp theo
       readChar();
       
       if (currentChar == EOF) {
         // Gặp EOF ngay sau '*' → lỗi
         error(ERR_ENDOFCOMMENT, ln, cn);
         return;
       }
       
       if (charCodes[currentChar] == CHAR_RPAR) {
         // Gặp ')' sau '*' → kết thúc comment
         readChar(); // Bỏ qua ')'
         return;
       }
       // Nếu không phải ')', tiếp tục đọc (đã readChar rồi)
     } else {
       // Ký tự thường, tiếp tục đọc
       readChar();
     }
   }
   
   // Nếu thoát vòng lặp = gặp EOF mà chưa đóng comment
   // Báo lỗi ở vị trí của '(' mở đầu, nhưng thực tế expected muốn báo ở vị trí EOF
   error(ERR_ENDOFCOMMENT, lineNo, colNo);
 }
 Token* readIdentKeyword(void) {
   // Khi vào hàm này, currentChar là CHAR_LETTER (ký tự đầu tiên)
   // Mục tiêu: đọc identifier/keyword và phân loại
   
   Token *token = makeToken(TK_IDENT, lineNo, colNo);
   int count = 0;
   
   // Đọc tất cả các ký tự letter và digit
   while (currentChar != EOF && 
          (charCodes[currentChar] == CHAR_LETTER || 
           charCodes[currentChar] == CHAR_DIGIT)) {
     
     // Chỉ lưu nếu chưa vượt quá giới hạn
     if (count < MAX_IDENT_LEN) {
       token->string[count++] = (char)currentChar;
     }
     // Vẫn phải readChar() để đọc hết identifier
     readChar();
   }
   
   // Kết thúc chuỗi
   if (count <= MAX_IDENT_LEN) {
     token->string[count] = '\0';
   } else {
     // Quá dài: chỉ lấy MAX_IDENT_LEN ký tự đầu
     token->string[MAX_IDENT_LEN] = '\0';
     error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
     return token;
   }
   
   // Kiểm tra xem có phải từ khóa không
   TokenType keywordType = checkKeyword(token->string);
   if (keywordType != TK_NONE) {
     token->tokenType = keywordType;
   }
   
   return token;
 }
 
 Token* readNumber(void) {
   // Khi vào hàm này, currentChar là CHAR_DIGIT (chữ số đầu tiên)
   // Mục tiêu: đọc số nguyên và chuyển đổi thành integer
   
   Token *token = makeToken(TK_NUMBER, lineNo, colNo);
   int count = 0;
   
   // Đọc tất cả các chữ số liên tiếp
   while (currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT) {
     
     // Chỉ lưu nếu chưa vượt quá giới hạn
     if (count < MAX_IDENT_LEN) {
       token->string[count++] = (char)currentChar;
     }
     // Vẫn phải readChar() để đọc hết số
     readChar();
   }
   
   // Kết thúc chuỗi
   if (count <= MAX_IDENT_LEN) {
     token->string[count] = '\0';
     token->value = atoi(token->string);
   } else {
     // Quá dài: chỉ lấy MAX_IDENT_LEN chữ số đầu
     token->string[MAX_IDENT_LEN] = '\0';
     token->value = atoi(token->string);
     error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
     return token;
   }
   
   return token;
 }
 
 Token* readConstChar(void) {
   // Khi vào hàm này, currentChar = '\'' (single quote mở đầu)
   // Format hợp lệ: 'x' (một ký tự bất kỳ giữa 2 dấu ')
   // Mục tiêu: đọc character constant và validate
   
   Token *token = makeToken(TK_CHAR, lineNo, colNo);
   
   readChar(); // Bỏ qua dấu '\'' mở đầu
   
   // Kiểm tra EOF ngay sau dấu '\'' mở
   if (currentChar == EOF) {
     error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
     return token;
   }
   
   // Lưu ký tự (chỉ 1 ký tự duy nhất)
   token->string[0] = (char)currentChar;
   token->string[1] = '\0';
   
   readChar(); // Đọc ký tự tiếp theo (phải là '\'' đóng)
   
   // Kiểm tra phải có dấu '\'' đóng
   if (currentChar == EOF || charCodes[currentChar] != CHAR_SINGLEQUOTE) {
     error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
     return token;
   }
   
   readChar(); // Bỏ qua dấu '\'' đóng
   
   return token;
 }
 
 Token* getToken(void) {
   Token *token;
   int ln, cn;
 
   if (currentChar == EOF) 
     return makeToken(TK_EOF, lineNo, colNo);
 
   switch (charCodes[currentChar]) {
   case CHAR_SPACE: skipBlank(); return getToken();
   case CHAR_LETTER: return readIdentKeyword();
   case CHAR_DIGIT: return readNumber();
   case CHAR_PLUS: 
     token = makeToken(SB_PLUS, lineNo, colNo);
     readChar(); 
     return token;
   case CHAR_MINUS:
     token = makeToken(SB_MINUS, lineNo, colNo);
     readChar();
     return token;
   case CHAR_TIMES:
     token = makeToken(SB_TIMES, lineNo, colNo);
     readChar();
     return token;
   case CHAR_SLASH:
     token = makeToken(SB_SLASH, lineNo, colNo);
     readChar();
     return token;
   case CHAR_LT:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
       readChar();
       return makeToken(SB_LE, ln, cn); // <=
     } else {
       return makeToken(SB_LT, ln, cn); // <
     }
   case CHAR_GT:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
       readChar();
       return makeToken(SB_GE, ln, cn); // >=
     } else {
       return makeToken(SB_GT, ln, cn); // >
     }
   case CHAR_EXCLAIMATION:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
       readChar();
       return makeToken(SB_NEQ, ln, cn); // !=
     } else {
       token = makeToken(TK_NONE, ln, cn);
       error(ERR_INVALIDSYMBOL, ln, cn);
       return token;
     }
   case CHAR_EQ:
     token = makeToken(SB_EQ, lineNo, colNo);
     readChar();
     return token;
   case CHAR_COMMA:
     token = makeToken(SB_COMMA, lineNo, colNo);
     readChar();
     return token;
   case CHAR_PERIOD:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF && charCodes[currentChar] == CHAR_RPAR) {
       readChar();
       return makeToken(SB_RSEL, ln, cn); // .)
     } else {
       return makeToken(SB_PERIOD, ln, cn); // .
     }
   case CHAR_COLON:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF && charCodes[currentChar] == CHAR_EQ) {
       readChar();
       return makeToken(SB_ASSIGN, ln, cn); // :=
     } else {
       return makeToken(SB_COLON, ln, cn); // :
     }
   case CHAR_SEMICOLON:
     token = makeToken(SB_SEMICOLON, lineNo, colNo);
     readChar();
     return token;
   case CHAR_SINGLEQUOTE:
     return readConstChar();
   case CHAR_LPAR:
     ln = lineNo;
     cn = colNo;
     readChar();
     if (currentChar != EOF) {
       if (charCodes[currentChar] == CHAR_TIMES) {
         skipComment(ln, cn);
         return getToken();
       } else if (charCodes[currentChar] == CHAR_PERIOD) {
         readChar();
         return makeToken(SB_LSEL, ln, cn); // (.
       }
     }
     return makeToken(SB_LPAR, ln, cn); // (
   case CHAR_RPAR:
     token = makeToken(SB_RPAR, lineNo, colNo);
     readChar();
     return token;
   default:
     token = makeToken(TK_NONE, lineNo, colNo);
     error(ERR_INVALIDSYMBOL, lineNo, colNo);
     readChar(); 
     return token;
   }
 }
 
 
 /******************************************************************/
 
 void printToken(Token *token) {
 
   printf("%d-%d:", token->lineNo, token->colNo);
 
   switch (token->tokenType) {
   case TK_NONE: printf("TK_NONE\n"); break;
   case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
   case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
   case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
   case TK_EOF: printf("TK_EOF\n"); break;
 
   case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
   case KW_CONST: printf("KW_CONST\n"); break;
   case KW_TYPE: printf("KW_TYPE\n"); break;
   case KW_VAR: printf("KW_VAR\n"); break;
   case KW_INTEGER: printf("KW_INTEGER\n"); break;
   case KW_CHAR: printf("KW_CHAR\n"); break;
   case KW_ARRAY: printf("KW_ARRAY\n"); break;
   case KW_OF: printf("KW_OF\n"); break;
   case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
   case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
   case KW_BEGIN: printf("KW_BEGIN\n"); break;
   case KW_END: printf("KW_END\n"); break;
   case KW_CALL: printf("KW_CALL\n"); break;
   case KW_IF: printf("KW_IF\n"); break;
   case KW_THEN: printf("KW_THEN\n"); break;
   case KW_ELSE: printf("KW_ELSE\n"); break;
   case KW_WHILE: printf("KW_WHILE\n"); break;
   case KW_DO: printf("KW_DO\n"); break;
   case KW_FOR: printf("KW_FOR\n"); break;
   case KW_TO: printf("KW_TO\n"); break;
 
   case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
   case SB_COLON: printf("SB_COLON\n"); break;
   case SB_PERIOD: printf("SB_PERIOD\n"); break;
   case SB_COMMA: printf("SB_COMMA\n"); break;
   case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
   case SB_EQ: printf("SB_EQ\n"); break;
   case SB_NEQ: printf("SB_NEQ\n"); break;
   case SB_LT: printf("SB_LT\n"); break;
   case SB_LE: printf("SB_LE\n"); break;
   case SB_GT: printf("SB_GT\n"); break;
   case SB_GE: printf("SB_GE\n"); break;
   case SB_PLUS: printf("SB_PLUS\n"); break;
   case SB_MINUS: printf("SB_MINUS\n"); break;
   case SB_TIMES: printf("SB_TIMES\n"); break;
   case SB_SLASH: printf("SB_SLASH\n"); break;
   case SB_LPAR: printf("SB_LPAR\n"); break;
   case SB_RPAR: printf("SB_RPAR\n"); break;
   case SB_LSEL: printf("SB_LSEL\n"); break;
   case SB_RSEL: printf("SB_RSEL\n"); break;
   }
 }
 
 int scan(char *fileName) {
   Token *token;
 
   if (openInputStream(fileName) == IO_ERROR)
     return IO_ERROR;
 
   token = getToken();
   while (token->tokenType != TK_EOF) {
     printToken(token);
     free(token);
     token = getToken();
   }
 
   free(token);
   closeInputStream();
   return IO_SUCCESS;
 }
 
 /******************************************************************/
 
 #ifndef UNIT_TEST
 int main(int argc, char *argv[]) {
     if (argc <= 1) {
       printf("scanner: no input file.\n");
       return -1;
     }
   
     if (scan(argv[1]) == IO_ERROR) {
       printf("Can\'t read input file!\n");
       return -1;
     }
       
     return 0;
 }
 #endif