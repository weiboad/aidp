--
--
-- 该脚本实现将 kafka 中的数据落地到文件中，并且统计落地消息数
--
--
local obj = aidp.message.get()
local filename = os.date('%Y-%m-%d-%H') .. '.log'
local files = {};
local storage = aidp.storage()
for k,v in pairs(obj) do
	if #v == 3 then
		if files[v[2]] == nil then
			files[v[2]] = io.open(v[2]..filename, 'a+');
			files[v[2]]:setvbuf('full')
		end	
		files[v[2]]:write(v[3]..'\n')
		storage:incr(v[2] .. ':message_size', 1)
	end
end
for k,v in pairs(files) do
	v:flush()
	v:close()
end
