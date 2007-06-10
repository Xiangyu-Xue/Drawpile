/*******************************************************************************

   Copyright (C) 2007 M.K.A. <wyrmchild@users.sourceforge.net>

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#include "shared/SHA1.h"

#include <iostream>
#include <memory>

int main()
{
	SHA1 hash;
	
	unsigned char src2[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	
	hash.Update(src2, strlen(reinterpret_cast<char*>(src2)));
	hash.Final();
	
	char hexdigest[41];
	hash.HexDigest(hexdigest);
	hexdigest[40] = '\0';
	
	char res2[] = "84983E441C3BD26EBAAE4AA1F95129E5E54670F1";
	
	int rv = memcmp(hexdigest, res2, 40);
	if (rv != 0)
		std::cerr << "result  : " << hexdigest << std::endl << "expected: " << res2 << std::endl;
	
	return (rv == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
