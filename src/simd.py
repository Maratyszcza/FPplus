class SimdOperations:
	def __init__(self, simd):
		assert simd in ["avx", "mic"]
		self.name = simd
		self.width = {"avx": 4, "mic": 8}[simd]
		self.regs = {"avx": 16, "mic": 32}[simd]
		self.dvec = {"avx": "__m256d", "mic": "__m512d"}[simd]
		self.ddvec = {"avx": "__m256dd", "mic": "__m512dd"}[simd]
		self._dzero = {"avx": "_mm256_setzero_pd", "mic": "_mm512_setzero_pd"}[simd]
		self._dload = {"avx": "_mm256_load_pd", "mic": "_mm512_load_pd"}[simd]
		self._dadd = {"avx": "_mm256_add_pd", "mic": "_mm512_add_pd"}[simd]
		self._dmul = {"avx": "_mm256_mul_pd", "mic": "_mm512_mul_pd"}[simd]
		self._dfma = {"avx": "_mm256_fmadd_pd", "mic": "_mm512_fmadd_pd"}[simd]
		self.ddzero = {"avx": "_mm256_setzero_pdd", "mic": "_mm512_setzero_pdd"}[simd]
		self._ddadd = {"avx": "_mm256_add_pdd", "mic": "_mm512_add_pdd"}[simd]
		self._ddmul = {"avx": "_mm256_mul_pdd", "mic": "_mm512_mul_pdd"}[simd]
		self._defadd = {"avx": "_mm256_efadd_pd", "mic": "_mm512_efadd_pd"}[simd]
		self._defmul = {"avx": "_mm256_efmul_pd", "mic": "_mm512_efmul_pd"}[simd]
		self.ddloaddeinterleave = {"avx": "_mm256_loaddeinterleave_pdd", "mic": "_mm512_loaddeinterleave_pdd"}[simd]
		self.ddloadudeinterleave = {"avx": "_mm256_loadudeinterleave_pdd", "mic": "_mm512_loadudeinterleave_pdd"}[simd]
		self.ddinterleavestore = {"avx": "_mm256_interleavestore_pdd", "mic": "_mm512_interleavestore_pdd"}[simd]
		self.ddinterleavestoreu = {"avx": "_mm256_interleavestoreu_pdd", "mic": "_mm512_interleavestoreu_pdd"}[simd]
		self.ddbroadcast = {"avx": "_mm256_broadcast_sdd", "mic": "_mm512_broadcast_sdd"}[simd]


	def dzero(self):
		return self._dzero + "()"

	def dload(self, addr):
		return self._dload + "(" + str(addr) + ")"

	def ddadd(self, a, b):
		return self._ddadd + "(" + str(a) + ", " + str(b) + ")"

	def ddmul(self, a, b):
		return self._ddmul + "(" + str(a) + ", " + str(b) + ")"
