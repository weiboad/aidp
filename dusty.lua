print("sssssas")
local obj = aidp.message.get()
local filename = 'test' .. os.date('%Y-%m-%d-%H') .. '.log'
local file = io.open(filename, 'a+')
file:setvbuf('full')
for k,v in pairs(obj) do
	if #v == 2 then
		print(v[2])
		file:write(v[2]..'\n')
	end
end
file:flush()
file:close()
