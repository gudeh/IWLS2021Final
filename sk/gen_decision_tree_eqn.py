def gen_header(fHandler):
	print("INORDER = ", file=fHandler)
	for i in range(3072):
		for j in range(8):
			print("feat_%d_%d" % (i, j), file=fHandler)
	print(";", file=fHandler)
	print("", file=fHandler)
    
	print("OUTORDER = ", file=fHandler)
	for i in range(10):
		print("decision_%d" % (i), file=fHandler)
	print(";", file=fHandler)
	print("", file=fHandler)
    
def genCte_eqn(fHandler, signal, constValue):
	size = len(constValue)
	for idx in range(size):
		if constValue[idx] == '0':
			print("%s_%d = 0;" % (signal, 8 - idx - 1), file=fHandler)
		else:
			print("%s_%d = 1;" % (signal, 8 - idx - 1), file=fHandler)
            
            
def halfAdder_eqn(fHandler, inA, inB, outS, outC):
	print("%s = (%s + %s) * !(%s * %s);" % (outS, inA, inB, inA, inB), file = fHandler)
	print("%s = %s * %s;" % (outC, inA, inB), file = fHandler)

def fullAdder_eqn(fHandler, inA, inB, inC, outS, outC):
	print("%s_%s_%s_partial = (%s + %s) * !(%s * %s);" % (inA, inB, inC, inA, inB, inA, inB), file = fHandler)
	print("%s = (%s_%s_%s_partial * %s) + (%s * %s);" % (outC, inA, inB, inC, inC, inA, inB), file = fHandler)
	print("%s = (%s_%s_%s_partial + %s) * !(%s_%s_%s_partial * %s);" % (outS, inA, inB, inC, inC, inA, inB, inC, inC), file = fHandler)

    
def fullAdderNoCout_eqn(fHandler, inA, inB, inC, outS):
	print("%s_%s_%s_partial = (%s + %s) * !(%s * %s);" % (inA, inB, inC, inA, inB, inA, inB), file = fHandler)
	print("%s = (%s_%s_%s_partial + %s) * !(%s_%s_%s_partial * %s);" % (outS, inA, inB, inC, inC, inA, inB, inC, inC), file = fHandler)


def invSignal_eqn(fHandler, inA, outS, size):
	for i in range(size):
		print("%s_%d = !(%s_%d);" % (outS, i, inA, i), file = fHandler)
        
   
def invSignal_msb_eqn(fHandler, inA, outS, size):
	for i in range(size):
		print("%s_%d = !(%s_%d);" % (outS, 8 - i - 1, inA, 8 - i - 1), file = fHandler)
        

def twosComp_eqn(fHandler, inA, outS, size):
	invSignal_eqn(fHandler, inA, "inv_%s" % (inA), size)



def genAdder_eqn(fHandler, inA, inB, outS, size):
	halfAdder_eqn(fHandler, "%s_0" % (inA), "%s_0" % (inB), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
	for i in range(1, size-1):
		fullAdder_eqn(fHandler, "%s_%d" % (inA, i), "%s_%d" % (inB, i), "%s_%s_c_%d" % (inA, inB, i-1), "%s_%d" % (outS, i), "%s_%s_c_%d" % (inA, inB, i))

	fullAdder_eqn(fHandler, "%s_%d" % (inA, size-1), "%s_%d" % (inB, size-1), "%s_%s_c_%d" % (inA, inB, size-2), "%s_%d" % (outS, size-1), "%s_%d" % (outS, size))
        

def genAdder_generic_eqn(fHandler, inA, inB, outS, size):
	if size > 2:
		halfAdder_eqn(fHandler, "%s_%d" % (inA, 8 - size), "%s_%d" % (inB, 8 - size), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
		cIdx = 0
		for i in range(8 - size + 1, 7):
			fullAdder_eqn(fHandler, "%s_%d" % (inA, i), "%s_%d" % (inB, i), "%s_%s_c_%d" % (inA, inB, cIdx), "%s_%d" % (outS, cIdx+1), "%s_%s_c_%d" % (inA, inB, cIdx+1))
			cIdx += 1

		fullAdder_eqn(fHandler, "%s_%d" % (inA, 7), "%s_%d" % (inB, 7), "%s_%s_c_%d" % (inA, inB, cIdx), "%s_%d" % (outS, cIdx + 1), "%s_%d" % (outS, cIdx + 2))
	elif size == 2:
		halfAdder_eqn(fHandler, "%s_%d" % (inA, 8 - size), "%s_%d" % (inB, 8 - size), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
		fullAdder_eqn(fHandler, "%s_%d" % (inA, 8 - size + 1), "%s_%d" % (inB, 8 - size + 1), "%s_%s_c_%d" % (inA, inB, 0), "%s_%d" % (outS, 1), "%s_%d" % (outS, 2))
	else:
		halfAdder_eqn(fHandler, "%s_%d" % (inA, 8 - size), "%s_%d" % (inB, 8 - size), "%s_0" % (outS), "%s_1" % (inA, inB))
        
        
def genComp_eqn(fHandler, inA, inB, outS, size):

	invSignal_eqn(fHandler, inB, "%s_inv" % (inB), size)
	genAdder_eqn(fHandler, inA, "%s_inv" % (inB), "%s_%s_sub" % (inA, inB), size)
	print("%s = !(%s_%s_sub_%d);" % (outS, inA, inB, size), file = fHandler)
    

def genComp_generic_eqn(fHandler, inA, inB, outS, size):

	invSignal_msb_eqn(fHandler, inB, "%s_inv" % (inB), size)
	genAdder_generic_eqn(fHandler, inA, "%s_inv" % (inB), "%s_%s_sub" % (inA, inB), size)
	print("%s = !(%s_%s_sub_%d);" % (outS, inA, inB, size), file = fHandler)


def genAdderNoCout_eqn(fHandler, inA, inB, outS, size):    
	if size > 2:
		halfAdder_eqn(fHandler, "%s_0" % (inA), "%s_0" % (inB), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
		for i in range(1, size-1):
			fullAdder_eqn(fHandler, "%s_%d" % (inA, i), "%s_%d" % (inB, i), "%s_%s_c_%d" % (inA, inB, i-1), "%s_%d" % (outS, i), "%s_%s_c_%d" % (inA, inB, i))

		fullAdderNoCout_eqn(fHandler, "%s_%d" % (inA, size-1), "%s_%d" % (inB, size-1), "%s_%s_c_%d" % (inA, inB, size-2), "%s_%d" % (outS, size-1))
	elif size == 2:
		halfAdder_eqn(fHandler, "%s_0" % (inA), "%s_0" % (inB), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
		fullAdderNoCout_eqn(fHandler, "%s_%d" % (inA, size-1), "%s_%d" % (inB, size-1), "%s_%s_c_%d" % (inA, inB, size-2), "%s_%d" % (outS, size-1))
	else:
		halfAdder_eqn(fHandler, "%s_0" % (inA), "%s_0" % (inB), "%s_0" % (outS), "%s_%s_c_0" % (inA, inB))
    
	print("", file = fHandler)
    
def extendSignal_eqn(fHandler, inA, oldSize, newSize):
	print("%s_0 = %s;" % (inA, inA), file = fHandler)
	for i in range(oldSize, newSize+1):
		print("%s_%d = 0;" % (inA, i), file = fHandler)
	
def concatSignal_eqn(fHandler, outS, insA):
	for idx, elem in enumerate(insA):
		print("%s_%d = %s;" % (outS, idx, elem), file = fHandler)
    
def trnsfrmLargeSelIn1bit_eqn(fHandler, sel, outSel, valueToEqual):
	valuesToAnd = []
	for idx, value in enumerate(valueToEqual):
		if value == '0':
			valuesToAnd.append("!(%s_%d)" % (sel, idx))
		else:
			valuesToAnd.append("(%s_%d)" % (sel, idx))
	print("%s = %s;" % (outSel, " * ".join(valuesToAnd)), file = fHandler)
      
def attribSignal_eqn(fHandler, inA, outS):
	print("%s = %s;" % (inA, outS), file = fHandler)
    
def genCompHeq_eqn(fHandler, inA, inB, outS, size):
	invSignal_eqn(fHandler, inB, "%s_inv" % (inB), size)
	genAdder_eqn(fHandler, inA, "%s_inv" % (inB), "%s_%s_sub" % (inA, inB), size)
	print("is_equal_%s_%s = %s;" % (inA, inB, " * ".join(["%s_%s_sub_%d" % (inA, inB, x) for x in range(size)])), file = fHandler)
	print("%s = is_equal_%s_%s + %s_%s_sub_%d;" % (outS, inA, inB, inA, inB, size), file = fHandler)

def mux21_eqn(fHandler, inA, inB, sel, outS, size):
	for idx in range(size):
		print("%s_%d = (%s_%d * %s) + (%s_%d * !%s);" % (outS, idx, inA, idx, sel, inB, idx, sel), file = fHandler)