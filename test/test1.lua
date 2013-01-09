require("luapb");

pb.import("test.proto");

--required int32 uid = 1;
--	required int32 param = 2;
--	optional string param1 = 3;
local msg = pb.new("lm.test");
msg.uid = 12345;
msg.param = 9876;
msg.param1 = "zjx";
--print("type: " .. type(msg.param2));
msg.param2.add("first");
msg.param2.add("second");

msg.param2.add("three");

print("uid: " .. msg.uid);
print("param: " .. msg.param);
print("param1: " .. msg.param1);

for i = 1, msg.param2.len() do
	local value = msg.param2[i];
	print("i: " .. i .. " value: " .. value); 
end
print("str: " .. pb.tostring(msg));
print("hello world");
