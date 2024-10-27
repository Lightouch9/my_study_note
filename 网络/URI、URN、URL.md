# URI

`URI`全称`Uniform Resource Identifier`，即统一资源标识符，提供一个资源的唯一标识，包括该资源的名字、位置等信息。但并不提供对于资源的访问方法。

`URI`是一个广泛的概念，其包括`URN`和`URL`。

# URN

`URN`全称`Uniform Resource Name`，即统一资源名称，用于标识资源，只为资源命名而不定位资源，不提供对资源的访问方法，例如：

```
ISBN书籍编号
urn:isbn:9787121411748
```

`URN`使用得比较少，几乎使用的所有`URI`都是`URL`，因此多数情况下对于一般的网络链接即可称之为`URL`也可称之为`URI`。

# URL

`URL`全称`Uniform Resource Locator`，即统一资源定位符，不仅标识了一个资源还提供了访问该资源的方法，如访问协议、访问路径、资源名称等信息。

## 格式

`URL`需要遵循一定的格式规范：

```
scheme://[username:password@]hostname[:port][/path][;parameters][?query][#fragment]
```

`[ ]`内容表示非必要部分。

- `scheme`：协议，也可称之为`protocol`，指定了访问资源所使用的协议或方法，常见协议有`http`、`https`、`ftp`等。
  - `http://`
  - `ftp://`
- `username`、`password`：用户名与密码，可选，某些情况下需要提供用户名和密码才能访问，假如`http://abc.com`需要用户名密码才能访问，则可通过`http://username:userpasswd@abc.com`直接访问。
- `hostname`：主机名或主机地址。提供资源的服务器的域名或ip地址，通常情况下是使用域名。比如`https://www.baidu.com`中`www.baidu.com`就是该`URL`的`hostname`，这里是域名，而`https://192.168.1.1`的`hostname`就是`192.168.1.1`，这里是ip地址。
- `port`：端口号，指定访问的服务器的服务的端口，可选，未填写时则使用访问协议的默认端口，`http`默认端口`80`，`https`默认端口`443`。`https://abc.com:80`中`port`即为`80`，而未指定端口的`https://abc.com`等价于`https://abc.com:443`。
- `path`：路径，可选，指定资源在服务器中的访问路径，通常是一个目录或文件的路径。如`https://abc.com/index.html`访问的路径就是根目录下的`index.html`文件。
- `parameters`：参数，可选，指定访问某个资源时的附加信息，如`https://abc.com/hello;user`，`user`即为附加信息，`parameters`字段不常用。
- `query`：查询参数，可选，用于查询某类资源，如有多个查询则用`&`分隔开。如`https://www.abc.com/search?id=123&lang=cn`，指定了查询参数`id`为`123`，`lang`为`cn`。 该参数使用频率很高。
- `fragment`：片段，可选，对资源描述的部分补充，指向资源中特定位置，类似于请求资源的内部书签，如`https://www.abc.com/index.html#section2`。

# 小结

`URN`与`URL`都属于`URI`，`URN`只标识资源的名称，`URL`不仅标识名称还指定了访问方法，所以使用更为广泛，目前几乎所有的`URI`都是`URL`，所以大多数情况下一般的网络链接即可称之为`URI`也可称之为`URL`。