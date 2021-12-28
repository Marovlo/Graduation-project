with open(r"C:\Users\Marovlo\OneDrive - bupt.edu.cn\工作文档\毕业设计\RFC3261-SIP.txt",'r')as f:
    with open(r"C:\Users\Marovlo\OneDrive - bupt.edu.cn\工作文档\毕业设计\RFC3261-SIP-new.txt",'w')as fn:
        for line in f.readlines():
            for word in line:
                if word!='':
                    fn.write(word)