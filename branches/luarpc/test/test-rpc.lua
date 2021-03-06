function error_handler (message)
	io.write ("MY ERROR: " .. message .. "\n");
end

rpc.on_error (error_handler);

slave,err = rpc.connect ("/dev/tty.usbserial-ftCYPMYJ");
--slave,err = rpc.connect ("/dev/ttys0");

print("Platform: " .. slave.pd.platform())
print("CPU: " .. slave.pd.cpu())
print("Board: " .. slave.pd.board())
print("CPU Clock: " .. slave.cpu.clock()/1000000 .. " MHz")

function mirror( input ) return input end
function squareval(x) return x*x end
test_local = {1, 2, 3, 4, "234"}
test_local.sval = 23

slave.mirror = mirror

-- print(slave.mirror:get())
-- reflect parameters off mirror
-- also requires that function serialization works
assert(slave.mirror(42) == 42, "integer return failed")
assert(slave.mirror("The quick brown fox jumps over the lazy dog") == "The quick brown fox jumps over the lazy dog", "string return failed")
assert(string.dump(slave.mirror(squareval)) == string.dump(squareval), "function return failed")
assert(slave.mirror(true) == true, "bool return failed")

slave.test = test_local

-- get remote table
assert(slave.test:get(), "couldn't get remote table")

-- check that we can get entry on remote table
assert(test_local.sval == slave.test:get().sval, "table field not equivalent")

adc = slave.adc

adc.setblocking(0,1)
adc.setclock(0, 64 ,2)

adc.sample(0,128)
-- print(slave.collectgarbage("count"))
for i=1,128 do
	retsamp = adc.getsample(0)
	if not (retsamp == nil) then
		print(retsamp)
	end
end
